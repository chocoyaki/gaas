#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#define TAG 42

using namespace std;

void print_matrix(double* matrix, int nb_lines, int nb_cols) {
	int index = 0;
	for(int li = 0; li < nb_lines; li++) {
		for(int col = 0; col < nb_cols; col++) {
			printf(" %f", matrix[index]);
			index++;
		}
		printf("\n");
	}
}

int write_matrix_file(const char* path, double* matrix, int nb_lines, int nb_cols) {
	FILE* file = fopen(path, "w");
	if (file == NULL) {
		printf("unable to open %s\n", path);
		return -1;
	}
	
	int index = 0;
	for(int li = 0; li < nb_lines; li++) {
		for(int col = 0; col < nb_cols; col++) {
			fprintf(file, " %f", matrix[index]);
			index++;
		}
		fprintf(file, "\n");
	}
	
	fclose(file);
}

inline
istream& operator>>(istream& In, std::string& L)
{
	return getline(In, L);
}



double* read_matrix_file(const char* path, int *nb_lines, int *nb_cols, bool read) {
	ifstream FichierMatrice(path,ios::in);
	if (!FichierMatrice)
	{
		cerr << "Ouverture du fichier impossible" << endl;
		return NULL;
	}

	vector<double> values;
	double val;
	*nb_lines = 0;
	
	// lecture de chaque ligne :
	string line;
	while (FichierMatrice >> line)
	{
		(*nb_lines)++;
		(*nb_cols) = 0;
		istringstream S(line, ios_base::in);
		
		// lecture de chaque element :
 		while (S >> val)
 		{
 			(*nb_cols)++;
   			cout << val << " ";
   			if (read) {
	   			values.push_back(val);
	   		}
 		}
 	
 		
	}
	
	int index = 0;
	double *A = (double *) calloc ((*nb_lines) * (*nb_cols), sizeof(double));
	
	if (read) {
		for(int index = 0; index < (*nb_cols) * (*nb_lines); index++){
				A[index] = values[index];
				index++;
		}
	}
	return A;
}


int main(int argc, char *argv[])
{
  int myrank, P, from, to, i, j, k;
  int tag = 666;		/* any value will do */
  MPI_Status status;
  
  if (argc < 4) {
  	printf("usage : %s A.txt B.txt out.txt\n", argv[0]);
  	exit(-1);
  }
  
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);	/* who am i */
  MPI_Comm_size(MPI_COMM_WORLD, &P); /* number of processors */

  /* Just to use the simple variants of MPI_Gather and MPI_Scatter we */
  /* impose that SIZE is divisible by P. By using the vector versions, */
  /* (MPI_Gatherv and MPI_Scatterv) it is easy to drop this restriction. */
	
	int SIZE;
	double* A;
	double* B;
	double* C;
  if (SIZE%P!=0) {
    if (myrank==0) printf("Matrix size not divisible by number of processors\n");
    MPI_Finalize();
    exit(-1);
  }



  

  /* Process 0 fills the input matrices and broadcasts them to the rest */
  /* (actually, only the relevant stripe of A is sent to each process) */


	const char* pathA = argv[1];
	const char* pathB = argv[2];
  	const char* pathC = argv[3];
	bool read_matrix_values;
	if (myrank==0) {
    	read_matrix_values = true;
    }
    else {
    	read_matrix_values = false;
    }
    
	int nb_lines_A, nb_cols_A;
	int nb_lines_B, nb_cols_B;

	A = read_matrix_file(pathA, &nb_lines_A, &nb_cols_A, read_matrix_values);
	if (A == NULL) {
		printf ("unable to load matrix A\n");
		exit(-1);
	}

	if (nb_lines_A != nb_cols_A) {
		printf ("matrix A is expected to be square\n");
		exit(-1);
	}

	B = read_matrix_file(pathB, &nb_lines_B, &nb_cols_B, read_matrix_values);
	if (B == NULL) {
		printf ("unable to load matrix B\n");
		exit(-1);
	}

	if (nb_lines_B != nb_cols_B) {
		printf ("matrix B is expected to be square\n");
		exit(-1);
	}    

	if ( nb_lines_A != nb_lines_B) {
		printf("matrices A and B are expected to be the same size\n");
		exit(-1);
	}
    
  	SIZE = nb_lines_A;
  
  	C = (double*) malloc(sizeof(double) * SIZE * SIZE);
	
	
	from = myrank * SIZE/P;
	to = (myrank+1) * SIZE/P;
	
  MPI_Bcast (B, SIZE*SIZE, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Scatter (A, SIZE*SIZE/P, MPI_DOUBLE, A+(from*SIZE), SIZE*SIZE/P, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  printf("computing slice %d (from row %d to %d)\n", myrank, from, to-1);
  for (i=from; i<to; i++) 
    for (j=0; j<SIZE; j++) {
      C[i * SIZE + j] = 0; //C[i][j]=0;
      for (k=0; k<SIZE; k++) {
		C[i * SIZE + j] += A[i * SIZE + k] * B[k * SIZE + j]; //C[i][j] += A[i][k]*B[k][j];
	  }
    }

  MPI_Gather (C + (from * SIZE), SIZE*SIZE/P, MPI_DOUBLE, C, SIZE*SIZE/P, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if (myrank==0) {
    printf("\n\n");
    print_matrix(A, SIZE, SIZE);
    printf("\n\n\t       * \n");
    print_matrix(B, SIZE, SIZE);
    printf("\n\n\t       = \n");
    print_matrix(C, SIZE, SIZE);
    printf("\n\n");
    
    int env = write_matrix_file(pathC, C, SIZE, SIZE);
    if (env) {
    	exit(-1);
    }
  }

  MPI_Finalize();
  return 0;
}


