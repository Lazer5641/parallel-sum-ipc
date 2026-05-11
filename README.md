# Parallel Sum Calculator

This is my Operating Systems project for process creation and IPC using `fork()` + pipes.

## Compile and run

This program is for Linux/POSIX systems.

```bash
g++ parallel_sum.cpp -o parallel_sum -Wall -std=c++17
./parallel_sum <N> <M>
```

Example:

```bash
./parallel_sum 1000 5
```

## What it does

- Reads `N` (vector size) and `M` (number of child processes).
- Checks input (`N > 0`, and `1 <= M <= 100`).
- Fills the vector with random float values in `[-100, 100]`.
- Splits work across children with:
  - `base = N / M`
  - `remainder = N % M`
  - first `remainder` children get one extra element
- Each child sums its range and sends one partial sum to parent through its own pipe.
- Parent reads all partial sums, waits for children, and prints final parallel sum.
- Program also calculates sequential sum and prints the difference for verification.
- On Linux, it also prints simple timing and speedup.

## Test cases used

I tested with:

```bash
./parallel_sum 100 4
./parallel_sum 100 3
./parallel_sum 1 5
```

Expected behavior:

- `N=100, M=4` -> even split (25 each)
- `N=100, M=3` -> split is 34, 33, 33
- `N=1, M=5` -> first child handles 1 item, rest get empty ranges

Output values change every run because numbers are random, but parallel/sequential sums should match within epsilon.

## Invalid input handling

- Missing args -> usage message
- `N <= 0` -> invalid vector size
- `M < 1` or `M > 100` -> invalid process count

## Notes / limitations

- Uses `fork`, `pipe`, and `waitpid`, so this is Linux/POSIX oriented.
- Uses `float`, so small precision differences are possible; that is why difference is printed.
- For very small `N`, process overhead may be larger than performance gain.

## Files in submission

- `parallel_sum.cpp`
- `PART1_ANSWERS.md`
