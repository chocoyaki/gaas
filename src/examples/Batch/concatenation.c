/**
 * @file concatenation.c
 *
 * @brief   DIET server for Batch submission
 *
 * @author   Yves Caniou (Yves.Caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include </usr/lib/mpich/include/mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define NB_ARGS 4
#define MAX_STRING_LENGTH 100
#define SLEEPING_TIME 30

int
main(argc, argv)
int argc;
char *argv[];

{
  int size, my_num;
  char chaine[MAX_STRING_LENGTH];
  FILE *output_file, *file;
  size_t count;

  MPI_Init(&argc, &argv);

  if (argc != NB_ARGS + 1) {
    fprintf(stderr, "%s requires %d arguments:\n"
                    "- a filename\n"
                    "- a string (which will be printed by all process)\n"
                    "- a filename\n"
                    "- the output file\n",
            argv[0],
            NB_ARGS);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &my_num);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  printf("Hello world from %d out of %d : %s\n", my_num, size, argv[2]);
  printf("%d sleeping for %d seconds\n\n", my_num, SLEEPING_TIME);

  if (my_num == 0) {
    file = fopen(argv[1], "r");
    output_file = fopen(argv[NB_ARGS], "w+");
    while (!feof(file)) {
      count = fread(&chaine, sizeof(char), MAX_STRING_LENGTH, file);
      fwrite(chaine, sizeof(char), count, output_file);
    }
    fclose(file);
    file = fopen(argv[3], "r");
    while (!feof(file)) {
      count = fread(&chaine, sizeof(char), MAX_STRING_LENGTH, file);
      fwrite(chaine, sizeof(char), count, output_file);
    }
    fclose(file);
    fclose(output_file);
  }

  sleep(SLEEPING_TIME);

  MPI_Finalize();
  exit(0);
}
