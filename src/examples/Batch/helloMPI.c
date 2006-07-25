#include </usr/lib/mpich/include/mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(argv, argc)
     int argv;
     char *argc[];

{
   int size, my_num;
   int nbsec = 5 ;
   
   MPI_Init(&argv, &argc);

   MPI_Comm_rank(MPI_COMM_WORLD, &my_num);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   
   printf("Hello world from %d out of %d : %s\n", my_num, size, argc[1]) ;
   printf("%d sleeping for %d seconds\n\n",my_num, nbsec) ;   
   
   MPI_Finalize();
   exit(0) ;

}
