#include <algorithm>
#include <array>
#include <cmath>
#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#ifdef __linux__
#include <time.h>
#endif

using namespace std;

float rand_float(float lo, float hi) {
    float t = (float)rand() / (float)RAND_MAX;
    return lo + t * (hi - lo);
}

void print_head(const vector<float>& v, int how_many) {
    int n = (int)v.size();
    int k = min(how_many, n);
    cout << "Vector values (first " << k << "): ";
    cout << fixed << setprecision(2);
    for (int i = 0; i < k; i++) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << endl;
}

float seq_sum(const vector<float>& v) {
    float s = 0;
    for (size_t i = 0; i < v.size(); i++)
        s += v[i];
    return s;
}

#ifdef __linux__
double now_sec() {
    timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}
#endif

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <vector_size> <num_processes>\n";
        return 1;
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    if (N <= 0) {
        cerr << "Error: N must be positive\n";
        return 1;
    }
    if (M < 1 || M > 100) {
        cerr << "Error: M should be between 1 and 100\n";
        return 1;
    }

    cout << "=== Parallel Sum Calculator ===\n";
    cout << "Vector size: " << N << "\n";
    cout << "Number of processes: " << M << "\n";

    srand((unsigned)time(NULL));

    vector<float> nums(N);
    for (int i = 0; i < N; i++)
        nums[i] = rand_float(-100.0f, 100.0f);

    print_head(nums, 10);

#ifdef __linux__
    double t0 = now_sec();
#endif
    float sequential = seq_sum(nums);
#ifdef __linux__
    double t1 = now_sec();
    double time_seq = t1 - t0;
#endif

    vector<array<int, 2>> pipes(M);
    for (int i = 0; i < M; i++) {
        if (pipe(pipes[i].data()) < 0) {
            cerr << "pipe failed on child " << i << ": " << strerror(errno) << endl;
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return 1;
        }
    }

    int base = N / M;
    int rem = N % M;

    vector<pid_t> pids(M);

    cout << "Creating " << M << " child processes...\n";

#ifdef __linux__
    double tp0 = now_sec();
#endif

    for (int i = 0; i < M; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            cerr << "fork failed: " << strerror(errno) << endl;
            for (int j = 0; j < M; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            for (int j = 0; j < i; j++)
                waitpid(pids[j], NULL, 0);
            return 1;
        }

        if (pid == 0) {
            for (int j = 0; j < M; j++) {
                close(pipes[j][0]);
                if (j != i)
                    close(pipes[j][1]);
            }

            int start = i * base + min(i, rem);
            int end = start + base + (i < rem ? 1 : 0);

            float part = 0;
            for (int j = start; j < end; j++)
                part += nums[j];

            ssize_t nwritten =
                write(pipes[i][1], &part, sizeof(float));
            if (nwritten != (ssize_t)sizeof(float)) {
                cerr << "write error in child\n";
                close(pipes[i][1]);
                _exit(1);
            }
            close(pipes[i][1]);
            _exit(0);
        }

        pids[i] = pid;
    }

    float parallel = 0;

    cout << fixed << setprecision(2);

    for (int i = 0; i < M; i++) {
        close(pipes[i][1]);

        float chunk_sum = 0;
        ssize_t nread = read(pipes[i][0], &chunk_sum, sizeof(float));
        if (nread != (ssize_t)sizeof(float)) {
            cerr << "read error from child " << i << endl;
            close(pipes[i][0]);
            for (int j = i + 1; j < M; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            for (int j = 0; j < M; j++)
                waitpid(pids[j], NULL, 0);
            return 1;
        }
        close(pipes[i][0]);

        int start = i * base + min(i, rem);
        int end = start + base + (i < rem ? 1 : 0);

        parallel += chunk_sum;
        waitpid(pids[i], NULL, 0);

        cout << "Child " << i << " (PID: " << pids[i] << "): ";
        if (start >= end)
            cout << "indices [empty], 0 elements = ";
        else
            cout << "indices [" << start << "-" << (end - 1) << "] = ";
        cout << chunk_sum << endl;
    }

#ifdef __linux__
    double tp1 = now_sec();
    double time_par = tp1 - tp0;
#endif

    float diff = fabs(parallel - sequential);

    cout << endl;
    cout << setprecision(2);
    cout << "Parallel sum: " << parallel << endl;
    cout << "Sequential sum: " << sequential << endl;
    cout << setprecision(6);
    cout << "Difference: " << diff << endl;

#ifdef __linux__
    cout << "Sequential time (s): " << time_seq << endl;
    cout << "Parallel section time (s): " << time_par << endl;
    if (time_par > 0)
        cout << "Speedup (sequential / parallel): " << (time_seq / time_par)
             << endl;
#endif

    const float EPS = 0.0001f;
    if (diff < EPS)
        cout << "Results match! Computation successful.\n";
    else
        cout << "Warning: sums don't match within epsilon.\n";

    return 0;
}