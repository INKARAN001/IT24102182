/* Exercise 2: Parallel sum of 1..10,000,000 using MPI
 * Each rank sums a contiguous block of the range, then the partial
 * sums are combined on rank 0 with MPI_Reduce.
 *
 * Compile: mpicc -O2 -o sum sum.c
 * Run:     mpiexec -n 4 ./sum
 */
#include <stdio.h>
#include <mpi.h>

#define N 10000000LL

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    /* split 1..N into 'size' blocks; the first (N % size) ranks get one extra */
    long long chunk = N / size;
    long long rem = N % size;
    long long first = rank * chunk + (rank < rem ? rank : rem) + 1;
    long long last = first + chunk - 1 + (rank < rem ? 1 : 0);

    long long local_sum = 0;
    for (long long i = first; i <= last; i++)
        local_sum += i;

    long long total = 0;
    MPI_Reduce(&local_sum, &total, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double elapsed = MPI_Wtime() - start;

    printf("Rank %d summed %lld..%lld = %lld\n", rank, first, last, local_sum);

    if (rank == 0) {
        printf("Total sum of 1..%lld = %lld (expected %lld)\n", N, total, N * (N + 1) / 2);
        printf("Processes: %d  Time: %f s\n", size, elapsed);
    }

    MPI_Finalize();
    return 0;
}
