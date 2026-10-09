/* Exercise 7: Exercise 6 rewritten with Buffered Send (MPI_Bsend)
 * Based on http://www.dartmouth.edu/~rc/classes/soft_dev/C_simple_ex.html
 *
 * Each rank throws NITER/size random points into the unit square and
 * counts how many fall inside the quarter circle. The workers send their
 * counts to rank 0 (MPI_Send / MPI_Recv), which computes
 *     pi ~= 4 * hits / NITER
 * Rank 0 receives with source = MPI_ANY_SOURCE, so it takes the counts in
 * whatever order the workers finish (status.MPI_SOURCE tells who sent it).
 * The final result is the same as Exercise 3 because addition does not
 * depend on the order; only the order of the "Received" lines changes.
 *
 * Workers send their counts with MPI_Bsend: the count is copied into an
 * attached buffer and Bsend returns at once, without waiting for rank 0
 * to post the matching receive.
 *
 * Compile: mpicc -O2 -o pi_bsend pi_bsend.c
 * Run:     mpiexec -n 4 ./pi_bsend
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define NITER 10000000LL
#define SEED 35791246

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    long long niter = NITER / size + (rank < NITER % size ? 1 : 0);

    /* each rank needs a different seed, otherwise all ranks
       generate the same points */
    srand(SEED + rank);

    long long count = 0;
    for (long long i = 0; i < niter; i++) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;
        if (x * x + y * y <= 1.0)
            count++;
    }

    if (rank != 0) {
        int bufsize;
        MPI_Pack_size(1, MPI_LONG_LONG, MPI_COMM_WORLD, &bufsize);
        bufsize += MPI_BSEND_OVERHEAD;
        char *buffer = malloc(bufsize);
        MPI_Buffer_attach(buffer, bufsize);

        MPI_Bsend(&count, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);

        /* detach waits until the buffered message has been sent */
        MPI_Buffer_detach(&buffer, &bufsize);
        free(buffer);
    } else {
        long long total = count;
        for (int i = 1; i < size; i++) {
            long long recv_count;
            MPI_Recv(&recv_count, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            printf("Received %lld hits from rank %d\n", recv_count, status.MPI_SOURCE);
            total += recv_count;
        }
        double pi = 4.0 * total / NITER;
        double elapsed = MPI_Wtime() - start;
        printf("# of trials = %lld, estimate of pi is %.10f (error %.2e)\n",
               NITER, pi, fabs(pi - M_PI));
        printf("Processes: %d  Time: %f s\n", size, elapsed);
    }

    MPI_Finalize();
    return 0;
}
