// Exercise 5.2: message2.cc rewritten with Buffered Send (MPI_Bsend)
// MPI_Bsend copies the data into a user-attached buffer and returns
// immediately, without waiting for the receiver. The send variable x is
// kept as is (it is not reused/overwritten to send data).
//
// Compile: mpicxx -o message2_bsend message2_bsend.cc
// Run:     mpiexec -n 4 ./message2_bsend      (needs at least 4 processes)
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
    int x[10], y[10];
    if (rank == 1) {
      for (int r =0;r <10; r++)
         x[r] = 10*r;

      // attach a buffer big enough for one message of 10 ints
      int bufsize;
      MPI_Pack_size(10, MPI_INT, MPI_COMM_WORLD, &bufsize);
      bufsize += MPI_BSEND_OVERHEAD;
      char *buffer = (char *)malloc(bufsize);
      MPI_Buffer_attach(buffer, bufsize);

      printf("Sending message to computer 3 from computer 1 (Bsend)\n");
      MPI_Bsend(x, 10, MPI_INT, 3, 0, MPI_COMM_WORLD);
      printf("Computer 1: Bsend returned (data copied to buffer)\n");

      // detach blocks until the buffered message has been delivered
      MPI_Buffer_detach(&buffer, &bufsize);
      free(buffer);
    }
    else if (rank == 3) {
      MPI_Recv(y, 10, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
      printf("in computer 3 the value of y is printed\n");
      for (int r=0;r<10;r++)
         printf(" %d ",y[r]);
      printf("\n");
    }
    else
      printf("Just a normal process From rank %d machine %s\n", rank, name);
    MPI_Finalize();
}
