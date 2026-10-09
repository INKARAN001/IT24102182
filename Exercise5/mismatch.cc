// Exercise 5.1: Source and destination do not match
// Modified from messages1.cc.
// Rank 1 sends to rank 3, but rank 3 expects the message from rank 2.
// MPI_Ssend blocks until a matching receive is posted, and MPI_Recv blocks
// until a matching message arrives - neither ever happens, so ranks 1 and 3
// wait forever (deadlock) and the program never finishes.
//
// Compile: mpicxx -o mismatch mismatch.cc
// Run:     mpiexec -n 4 ./mismatch      (needs at least 4 processes)
#include <cstdio>
#include <cstdlib>
#include <mpi.h>
int main(void)
{
    int rank;
    MPI_Status status;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    char name[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Get_processor_name( name, &len );
    int x, y;
    x = 30;
    if (rank == 1) {
      printf("Sending message to computer 3 from computer 1\n");
      fflush(stdout);
      MPI_Ssend(&x, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
      printf("Computer 1: send completed\n");   // never reached
    }
    else if (rank == 3) {
      printf("Computer 3 waiting for a message from computer 2\n");
      fflush(stdout);
      MPI_Recv(&y, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);  // wrong source
      printf("in computer 3 the value of y is %d\n", y);        // never reached
    }
    else
      printf("Just a normal process From rank %d machine %s\n", rank, name);
    fflush(stdout);
    MPI_Finalize();
}
