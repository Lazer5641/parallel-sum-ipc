# Parallel Sum with Processes and Pipes

A small Operating Systems project written in C++ that demonstrates parallel computation using POSIX child processes and inter-process communication.

The program creates multiple child processes with `fork()`, divides a vector of random floating-point values between them, collects partial sums through pipes, and compares the final parallel result with a sequential sum for verification.

## Concepts Demonstrated

- Process creation with `fork()`
- Inter-process communication using `pipe()`
- Parent/child process coordination with `waitpid()`
- Work partitioning across multiple processes
- Sequential vs. parallel result verification
- Basic timing and speedup measurement on Linux

## Requirements

This project is designed for Linux/POSIX environments.

You can run it on:

- Linux
- WSL on Windows
- macOS with minor timing-related differences

It will not compile in a normal Windows PowerShell or CMD environment because it uses POSIX system calls such as `fork()`, `pipe()`, and `waitpid()`.

## Build

```bash
g++ parallel_sum.cpp -o parallel_sum -Wall -std=c++17
```

## Run

```bash
./parallel_sum <vector_size> <num_processes>
```

Example:

```bash
./parallel_sum 1000 5
```

## How It Works

1. The program reads two command-line arguments:
   - `N`: vector size
   - `M`: number of child processes
2. It validates that `N > 0` and `1 <= M <= 100`.
3. It fills a vector with random `float` values in the range `[-100, 100]`.
4. It calculates the sequential sum as a reference result.
5. It splits the vector across `M` child processes:
   - `base = N / M`
   - `remainder = N % M`
   - the first `remainder` children receive one extra element
6. Each child computes the sum of its assigned range.
7. Each child sends its partial sum to the parent process through a pipe.
8. The parent collects all partial sums, waits for every child, and prints the final parallel sum.
9. The program compares the parallel sum with the sequential sum and prints the difference.

## Example Output

The exact numbers change on each run because the vector values are randomly generated.

```text
=== Parallel Sum Calculator ===
Vector size: 1000
Number of processes: 5
Vector values (first 10): -12.45, 83.10, 4.72, ...
Creating 5 child processes...
Child 0 (PID: 1234): indices [0-199] = 245.67
Child 1 (PID: 1235): indices [200-399] = -129.44
Child 2 (PID: 1236): indices [400-599] = 318.90
Child 3 (PID: 1237): indices [600-799] = -87.31
Child 4 (PID: 1238): indices [800-999] = 41.20

Parallel sum: 389.02
Sequential sum: 389.02
Difference: 0.000000
Results match! Computation successful.
```

## Test Cases

These are useful test cases for checking work distribution:

```bash
./parallel_sum 100 4
./parallel_sum 100 3
./parallel_sum 1 5
```

Expected distribution:

- `N = 100`, `M = 4`: each child handles 25 elements
- `N = 100`, `M = 3`: children handle 34, 33, and 33 elements
- `N = 1`, `M = 5`: the first child handles 1 element, and the rest receive empty ranges

## Input Validation

The program handles:

- Missing command-line arguments
- Invalid vector sizes where `N <= 0`
- Invalid process counts where `M < 1` or `M > 100`
- Pipe creation failures
- Fork failures
- Read/write errors between parent and child processes

## Limitations

- This is a learning project, not a production parallel computing library.
- Process creation overhead can be larger than the benefit for small vectors.
- Floating-point sums may have tiny precision differences, so the program compares results using an epsilon.
- The timing and speedup output is intended as a simple demonstration, not a formal benchmark.

## File Structure

```text
.
+-- README.md
`-- parallel_sum.cpp
```
