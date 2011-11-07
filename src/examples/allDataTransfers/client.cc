/**
<<<<<<< HEAD
 * @file client.cc
 *
 * @brief DIET all data transfers example: a client for transfering all data types
 *
 * @author Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

=======
* @file client.cc
*
* @brief DIET all data transfers example: a client for transfering all data types
*
* @author Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)
*
* @section Licence
*   |LICENSE|
*/
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7

#include <iostream>
#include <cstdlib>
#include <sys/stat.h>

#include "DIET_client.h"
#include "DIET_Dagda.h"
#include "progs.hh"


const unsigned int SIZE = 3;

static const unsigned int NB_PB = 6;
static const char *PB[NB_PB] =
{"CADD", "BADD", "IADD", "LADD", "FADD", "DADD"};

static const unsigned int NB_PB_VECTOR = 6;
static const char *PB_VECTOR[NB_PB_VECTOR] =
{"CVADD", "BVADD", "IVADD", "LVADD", "FVADD", "DVADD"};

static const unsigned int NB_PB_MATRIX = 6;
static const char *PB_MATRIX[NB_PB_MATRIX] =
{"CMADD", "BMADD", "IMADD", "LMADD", "FMADD", "DMADD"};

static const unsigned int NB_PB_STRING = 1;
static const char *PB_STRING[NB_PB_STRING] =
{"SPRINT"};

static const unsigned int NB_PB_PSTRING = 1;
static const char *PB_PSTRING[NB_PB_PSTRING] =
{"PSPRINT"};

static const unsigned int NB_PB_FILE = 1;
static const char *PB_FILE[NB_PB_FILE] =
{"FTRANSFER"};

static const unsigned int NB_PB_CONTAINER = 1;
static const char *PB_CONTAINER[NB_PB_CONTAINER] =
{"LCADD"};

/* argv[1]: client config file path */

void
usage(char *cmd) {
  std::cerr << "Usage: " << cmd << " <DIET_config.cfg> <file1>" << std::endl;
  exit(1);
}

int
main(int argc, char *argv[]) {
  unsigned int i;
  unsigned int errorPos = 0;
  diet_profile_t *profile = NULL;
  int error = 0;

  if (argc != 3) {
    usage(argv[0]);
  }

  /* Initialize DIET */
  if (diet_initialize(argv[1], argc, argv)) {
    std::cerr << "DIET initialization failed !" << std::endl;
    return 1;
  }

  /**
   * Scalar types
   */
  /* Use the long type for all "integer" types. */
  long l1 = 0;
  long l2 = 0;
  long *pl3 = NULL;
  /* It is not possible to apply this rule for floating types ... */
  float f1 = 0.0;
  float f2 = 0.0;
  float *pf3 = NULL;
  double d1 = 0.0;
  double d2 = 0.0;
  double *pd3 = NULL;

  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                            SCALARS                                 #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;
  /* Characters */
  l1 = static_cast<char>(0x1);
  l2 = static_cast<char>(0x2);
  std::cout << "#### Characters" << std::endl;
  std::cout << "Before the call: l1=" << (short) l1 << ", l2=" << (short) l2
            << std::endl;
  profile = diet_profile_alloc(PB[0], 0, 1, 2);
  diet_scalar_set(diet_parameter(profile, 0), &l1, DIET_VOLATILE, DIET_CHAR);
  diet_scalar_set(diet_parameter(profile, 1), &l2, DIET_VOLATILE, DIET_CHAR);
  diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_CHAR);
  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile, 2), &pl3, NULL);
    std::cout << "After the call: l1=" << (long) l1 << ", l2=" << (long) l2
              << ", l3=" << (long) *pl3 << std::endl;
  } else {
    std::cerr << "diet_call has returned with an error code on " << PB[0]
<<<<<<< HEAD
              << "!" << std::endl;
    error = error | (1 << errorPos);
=======
	      << "!" << std::endl;
    error = error | (1<<errorPos);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  }
  ++errorPos;
  diet_profile_free(profile);


  /* Short */
  l1 = (short) 0x11;
  l2 = (short) 0x22;
  std::cout << "#### Shorts" << std::endl;
  std::cout << "Before the call: l1=" << (short) l1 << ", l2=" << (short) l2
            << std::endl;
  profile = diet_profile_alloc(PB[1], 0, 1, 2);
  diet_scalar_set(diet_parameter(profile, 0), &l1, DIET_VOLATILE, DIET_SHORT);
  diet_scalar_set(diet_parameter(profile, 1), &l2, DIET_VOLATILE, DIET_SHORT);
  diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_SHORT);
  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile, 2), &pl3, NULL);
    std::cout << "After the call: l1=" << (long) l1 << ", l2=" << (long) l2
              << ", l3=" << (long) *pl3 << std::endl;
  } else {
    std::cerr << "diet_call has returned with an error code on " << PB[1]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);

  /* Integer */
  l1 = static_cast<int>(0x1111);
  l2 = static_cast<int>(0x2222);
  std::cout << "#### Integers" << std::endl;
  std::cout << "Before the call: l1=" << (int) l1 << ", l2="
<<<<<<< HEAD
            << static_cast<int>l2 << std::endl;
=======
	    << static_cast<int>(l2) << std::endl;
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  profile = diet_profile_alloc(PB[2], 0, 1, 2);
  diet_scalar_set(diet_parameter(profile, 0), &l1, DIET_VOLATILE, DIET_INT);
  diet_scalar_set(diet_parameter(profile, 1), &l2, DIET_VOLATILE, DIET_INT);
  diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_INT);
  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile, 2), &pl3, NULL);
    std::cout << "After the call: l1=" << (long) l1 << ", l2=" << (long) l2
              << ", l3=" << (long) *pl3 << std::endl;
  } else {
    std::cerr << "diet_call has returned with an error code on " << PB[2]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);


  /* Long */
  l1 = 0x11111111;
  l2 = 0x22222222;
  std::cout << "#### Longs" << std::endl;
  std::cout << "Before the call: l1=" << (long) l1 << ", l2=" << (long) l2
            << std::endl;
  profile = diet_profile_alloc(PB[3], 0, 1, 2);
  diet_scalar_set(diet_parameter(profile, 0), &l1, DIET_VOLATILE, DIET_LONGINT);
  diet_scalar_set(diet_parameter(profile, 1), &l2, DIET_VOLATILE, DIET_LONGINT);
  diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_LONGINT);
  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile, 2), &pl3, NULL);
    std::cout << "After the call: l1=" << (long) l1 << ", l2=" << (long) l2
              << ", l3=" << (long) *pl3 << std::endl;
  } else {
    std::cerr << "diet_call has returned with an error code on " << PB[3]
<<<<<<< HEAD
              << "!" << std::endl;
    error = error | (1 << errorPos);
=======
	      << "!" << std::endl;
    error = error | (1<<errorPos);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  }
  ++errorPos;
  diet_profile_free(profile);

  /* Float */
  f1 = 1.1e38;
  f2 = 2.2e38;
  std::cout << "#### Floats" << std::endl;
  std::cout << "Before the call: f1=" << f1 << ", f2=" << f2 << std::endl;
  profile = diet_profile_alloc(PB[4], 0, 1, 2);
  diet_scalar_set(diet_parameter(profile, 0), &f1, DIET_VOLATILE, DIET_FLOAT);
  diet_scalar_set(diet_parameter(profile, 1), &f2, DIET_VOLATILE, DIET_FLOAT);
  diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_FLOAT);
  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile, 2), &pf3, NULL);
<<<<<<< HEAD
    std::cout << "After the call: f1=" << static_cast<float> f1 << ", f2="
              << static_cast<float>f2 << ", f3=" << static_cast<float> *pf3
              << std::endl;
=======
    std::cout << "After the call: f1=" << static_cast<float>(f1) << ", f2="
	      << static_cast<float>(f2) << ", f3=" << static_cast<float>(*pf3)
	      << std::endl;
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  } else {
    std::cerr << "diet_call has returned with an error code on " << PB[4] <<
    "!"
              << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);

  /* Double */
  d1 = 1.1e307;
  d2 = 2.2e307;
  std::cout << "#### Doubles" << std::endl;
  std::cout << "Before the call: d1=" << d1 << ", d2=" << d2 << std::endl;
  profile = diet_profile_alloc(PB[5], 0, 1, 2);
  diet_scalar_set(diet_parameter(profile, 0), &d1, DIET_VOLATILE, DIET_DOUBLE);
  diet_scalar_set(diet_parameter(profile, 1), &d2, DIET_VOLATILE, DIET_DOUBLE);
  diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_DOUBLE);
  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile, 2), &pd3, NULL);
<<<<<<< HEAD
    std::cout << "After the call: d1=" << static_cast<float> d1 << ", l2="
              << static_cast<double>d2 << ", l3=" << static_cast<double> *pd3
              << std::endl;
  } else {
    std::cerr << "diet_call has returned with an error code on " << PB[5] <<
    "!"
              << std::endl;
    error = error | (1 << errorPos);
=======
    std::cout << "After the call: d1=" << static_cast<float>(d1) << ", l2="
	      << static_cast<double>(d2) << ", l3=" << static_cast<double>(*pd3)
	      << std::endl;
  } else {
    std::cerr << "diet_call has returned with an error code on " << PB[5] << "!"
	      << std::endl;
    error = error | (1<<errorPos);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  }
  ++errorPos;
  diet_profile_free(profile);
  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                            \\SCALARS                                #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;





  /**
   * Vector types
   */
  size_t n = SIZE;
  void *A = NULL;
  void *B = NULL;
  void *C = NULL;
  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                            VECTORS                                 #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;

  /* Use the long type for all "integer" types. */
  A = reinterpret_cast<long *>(malloc(sizeof(long) * SIZE));
  B = reinterpret_cast<long *>(malloc(sizeof(long) * SIZE));
  for (i = 0; i < SIZE; ++i) {
    (reinterpret_cast<long *>(A))[i] = i;
    (reinterpret_cast<long *>(B))[i] = i + SIZE;
  }

  /* Characters */
  std::cout << "#### Characters" << std::endl;
  profile = diet_profile_alloc(PB_VECTOR[0], 0, 1, 2);

  diet_vector_set(diet_parameter(profile, 0),
                  reinterpret_cast<long *>(A), DIET_VOLATILE, DIET_CHAR, n);
  std::cout << "A:" << std::endl;
  printVector((long *) A, n);
  diet_vector_set(diet_parameter(profile, 1),
                  reinterpret_cast<long *>(B), DIET_VOLATILE, DIET_CHAR, n);
  std::cout << "B (before call):" << std::endl;
  printVector(reinterpret_cast<long *>(B), n);
  diet_vector_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_CHAR, n);

  if (!diet_call(profile)) {
    diet_vector_get(diet_parameter(profile, 2),
<<<<<<< HEAD
                    reinterpret_cast<long **>(&C), NULL, &n);
=======
		    reinterpret_cast<long **>(&C), NULL, &n);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
    std::cout << "B (after call):" << std::endl;
    printVector(reinterpret_cast<long *>(B), n);
    std::cout << "C:" << std::endl;
    printVector(reinterpret_cast<long *>(C), n);
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on "
<<<<<<< HEAD
              << PB_VECTOR[0] << "!" << std::endl;
    error = error | (1 << errorPos);
=======
	      << PB_VECTOR[0] << "!" << std::endl;
    error = error | (1<<errorPos);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  }
  ++errorPos;
  diet_profile_free(profile);

  /* Short */
  for (i = 0; i < SIZE; ++i) {
    (reinterpret_cast<long *>(B))[i] = i + SIZE;
  }
  std::cout << "#### Shorts" << std::endl;
  profile = diet_profile_alloc(PB_VECTOR[1], 0, 1, 2);

  diet_vector_set(diet_parameter(profile, 0),
                  reinterpret_cast<long *>(A), DIET_VOLATILE, DIET_SHORT, n);
  std::cout << "A:" << std::endl;
  printVector(reinterpret_cast<long *>(A), n);
  diet_vector_set(diet_parameter(profile, 1),
                  reinterpret_cast<long *>(B), DIET_VOLATILE, DIET_SHORT, n);
  std::cout << "B (before call):" << std::endl;
  printVector(reinterpret_cast<long *>(B), n);
  diet_vector_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_SHORT, n);

  if (!diet_call(profile)) {
    diet_vector_get(diet_parameter(profile, 2),
<<<<<<< HEAD
                    reinterpret_cast<long **>(&C), NULL, &n);
=======
		    reinterpret_cast<long **>(&C), NULL, &n);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
    std::cout << "B (after call):" << std::endl;
    printVector(reinterpret_cast<long *>(B), n);
    std::cout << "C:" << std::endl;
    printVector(reinterpret_cast<long *>(C), n);
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on "
              << PB_VECTOR[1] << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);

  /* Integer */
  for (i = 0; i < SIZE; ++i) {
    (reinterpret_cast<long *>(B))[i] = i + SIZE;
  }
  std::cout << "#### Integers" << std::endl;
  profile = diet_profile_alloc(PB_VECTOR[2], 0, 1, 2);

  diet_vector_set(diet_parameter(profile, 0),
                  reinterpret_cast<long *> (A), DIET_VOLATILE, DIET_INT, n);
  std::cout << "A:" << std::endl;
  printVector(reinterpret_cast<long *>(A), n);
  diet_vector_set(diet_parameter(profile, 1),
                  reinterpret_cast<long *>(B), DIET_VOLATILE, DIET_INT, n);
  std::cout << "B (before call):" << std::endl;
  printVector(reinterpret_cast<long *>(B), n);
  diet_vector_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_INT, n);

  if (!diet_call(profile)) {
    diet_vector_get(diet_parameter(profile, 2),
<<<<<<< HEAD
                    reinterpret_cast<long **>(&C), NULL, &n);
=======
		    reinterpret_cast<long **>(&C), NULL, &n);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
    std::cout << "B (after call):" << std::endl;
    printVector(reinterpret_cast<long *>(B), n);
    std::cout << "C:" << std::endl;
    printVector(reinterpret_cast<long *>(C), n);
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
<<<<<<< HEAD
    std::cerr << "diet_call has returned with an error code on " <<
    PB_VECTOR[2]
              << "!" << std::endl;
    error = error | (1 << errorPos);
=======
    std::cerr << "diet_call has returned with an error code on " << PB_VECTOR[2]
	      << "!" << std::endl;
    error = error | (1<<errorPos);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  }
  ++errorPos;
  diet_profile_free(profile);

  /* Long */
  for (i = 0; i < SIZE; ++i) {
    (reinterpret_cast<long *>(B))[i] = i + SIZE;
  }
  std::cout << "#### Long" << std::endl;
  profile = diet_profile_alloc(PB_VECTOR[3], 0, 1, 2);

  diet_vector_set(diet_parameter(profile, 0),
                  reinterpret_cast<long *>(A), DIET_VOLATILE, DIET_LONGINT, n);
  std::cout << "A:" << std::endl;
  printVector(reinterpret_cast<long *>(A), n);
  diet_vector_set(diet_parameter(profile, 1),
                  reinterpret_cast<long *>(B), DIET_VOLATILE, DIET_LONGINT, n);
  std::cout << "B (before call):" << std::endl;
  printVector(reinterpret_cast<long *>(B), n);
  diet_vector_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_LONGINT, n);

  if (!diet_call(profile)) {
    diet_vector_get(diet_parameter(profile, 2), reinterpret_cast<long **>(&C),
<<<<<<< HEAD
                    NULL, &n);
=======
		    NULL, &n);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
    std::cout << "B (after call):" << std::endl;
    printVector(reinterpret_cast<long *>(B), n);
    std::cout << "C:" << std::endl;
    printVector(reinterpret_cast<long *>(C), n);
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
<<<<<<< HEAD
    std::cerr << "diet_call has returned with an error code on " <<
    PB_VECTOR[3]
              << "!" << std::endl;
    error = error | (1 << errorPos);
=======
    std::cerr << "diet_call has returned with an error code on " << PB_VECTOR[3]
	      << "!" << std::endl;
    error = error | (1<<errorPos);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  }
  ++errorPos;
  diet_profile_free(profile);
  free(A);
  free(B);

  /* Float */
  A = reinterpret_cast<float *>(malloc(sizeof(float) * SIZE));
  B = reinterpret_cast<float *>(malloc(sizeof(float) * SIZE));
  for (i = 0; i < SIZE; ++i) {
    (reinterpret_cast<float *>(A))[i] = i * 2.0;
    (reinterpret_cast<float *>(B))[i] = (i + SIZE) * 2.0;
  }
  std::cout << "#### Floats" << std::endl;
  profile = diet_profile_alloc(PB_VECTOR[4], 0, 1, 2);

  diet_vector_set(diet_parameter(profile, 0),
                  reinterpret_cast<float *>(A), DIET_VOLATILE, DIET_FLOAT, n);
  std::cout << "A:" << std::endl;
  printVector(reinterpret_cast<float *>(A), n);
  diet_vector_set(diet_parameter(profile, 1),
                  reinterpret_cast<float *>(B), DIET_VOLATILE, DIET_FLOAT, n);
  std::cout << "B (before call):" << std::endl;
  printVector(reinterpret_cast<float *>(B), n);
  diet_vector_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_FLOAT, n);

  if (!diet_call(profile)) {
    diet_vector_get(diet_parameter(profile, 2), reinterpret_cast<float **>(&C),
                    NULL, &n);
    std::cout << "B (after call):" << std::endl;
    printVector(reinterpret_cast<float *>(B), n);
    std::cout << "C:" << std::endl;
    printVector(reinterpret_cast<float *>(C), n);
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_VECTOR[4] << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);
  free(A);
  free(B);

  /* Double */
  A = (double *) malloc(sizeof(double) * SIZE);
  B = (double *) malloc(sizeof(double) * SIZE);
  for (i = 0; i < SIZE; ++i) {
    ((double *) A)[i] = i * 2.0;
    ((double *) B)[i] = (i + SIZE) * 2.0;
  }
  std::cout << "#### Doubles" << std::endl;
  profile = diet_profile_alloc(PB_VECTOR[5], 0, 1, 2);

  diet_vector_set(diet_parameter(profile, 0),
                  reinterpret_cast<float *>(A), DIET_VOLATILE, DIET_DOUBLE, n);
  std::cout << "A:" << std::endl;
  printVector(reinterpret_cast<float *>(A), n);
  diet_vector_set(diet_parameter(profile, 1),
                  reinterpret_cast<float *>(B), DIET_VOLATILE, DIET_DOUBLE, n);
  std::cout << "B (before call):" << std::endl;
  printVector(reinterpret_cast<float *>(B), n);
  diet_vector_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_DOUBLE, n);

  if (!diet_call(profile)) {
    diet_vector_get(diet_parameter(profile, 2), reinterpret_cast<float **>(&C),
                    NULL, &n);
    std::cout << "B (after call):" << std::endl;
    printVector(reinterpret_cast<float *>(B), n);
    std::cout << "C:" << std::endl;
    printVector(reinterpret_cast<float *>(C), n);
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_VECTOR[5]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);
  free(A);
  free(B);

  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                            \\VECTORS                                #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;





  /**
   * Matrix types
   */
  size_t m = SIZE;
  diet_matrix_order_t oA, oB, oC;
  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                            MATRICES                                #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;
  oA = DIET_ROW_MAJOR;
  oB = DIET_ROW_MAJOR;
  oC = DIET_ROW_MAJOR;



  /* Use the long type for all "integer" types. */
  A = reinterpret_cast<long *>(malloc(sizeof(long) * SIZE * SIZE));
  B = reinterpret_cast<long *>(malloc(sizeof(long) * SIZE * SIZE));
  for (i = 0; i < SIZE * SIZE; ++i) {
    (reinterpret_cast<long *>(A))[i] = i;
    (reinterpret_cast<long *>(B))[i] = i + SIZE;
  }

  /* Characters */
  std::cout << "#### Characters" << std::endl;
  profile = diet_profile_alloc(PB_MATRIX[0], 0, 1, 2);

  diet_matrix_set(diet_parameter(profile,
                                 0),
                  reinterpret_cast<long *>(A), DIET_VOLATILE, DIET_CHAR, m, n,
                  oA);
  std::cout << "A:" << std::endl;
  printMatrix(reinterpret_cast<long *>(A), m, n, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,
                                 1),
                  reinterpret_cast<long *>(B), DIET_VOLATILE, DIET_CHAR, n, m,
                  oB);
  std::cout << "B (before call):" << std::endl;
  printMatrix(reinterpret_cast<long *>(B), n, m, (oB == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_CHAR, m, m, oC);

  if (!diet_call(profile)) {
    diet_matrix_get(diet_parameter(profile, 2), reinterpret_cast<long **>(&C),
<<<<<<< HEAD
                    NULL, &m, &n, &oC);
=======
		    NULL, &m, &n, &oC);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
    std::cout << "B (after call):" << std::endl;
    printMatrix(reinterpret_cast<long *>(B), n, m, (oB == DIET_ROW_MAJOR));
    std::cout << "C:" << std::endl;
    printMatrix(reinterpret_cast<long *>(C), m, n, (oC == DIET_ROW_MAJOR));
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_MATRIX[0]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);



  /* Short */
  for (i = 0; i < SIZE * SIZE; ++i) {
    (reinterpret_cast<long *>(B))[i] = i + SIZE;
  }
  std::cout << "#### Shorts" << std::endl;
  profile = diet_profile_alloc(PB_MATRIX[1], 0, 1, 2);

  diet_matrix_set(diet_parameter(profile,
                                 0),
                  reinterpret_cast<long *>(A), DIET_VOLATILE, DIET_SHORT, m, n,
                  oA);
  std::cout << "A:" << std::endl;
  printMatrix(reinterpret_cast<long *>(A), m, n, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,
                                 1),
                  reinterpret_cast<long *>(B), DIET_VOLATILE, DIET_SHORT, n, m,
                  oB);
  std::cout << "B (before call):" << std::endl;
  printMatrix(reinterpret_cast<long *>(B), n, m, (oB == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_SHORT, m, m, oC);

  if (!diet_call(profile)) {
    diet_matrix_get(diet_parameter(profile, 2), reinterpret_cast<long **>(&C),
                    NULL, &m, &n, &oC);
    std::cout << "B (after call):" << std::endl;
    printMatrix(reinterpret_cast<long *>(B), n, m, (oB == DIET_ROW_MAJOR));
    std::cout << "C:" << std::endl;
    printMatrix(reinterpret_cast<long *>(C), m, n, (oC == DIET_ROW_MAJOR));
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_MATRIX[1]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);
  free(C);

  /* Integer */
  for (i = 0; i < SIZE * SIZE; ++i) {
    (reinterpret_cast<long *>(B))[i] = i + SIZE;
  }
  std::cout << "#### Integers" << std::endl;
  profile = diet_profile_alloc(PB_MATRIX[2], 0, 1, 2);

  diet_matrix_set(diet_parameter(profile,
                                 0),
                  reinterpret_cast<long *>(A), DIET_VOLATILE, DIET_INT, m, n,
                  oA);
  std::cout << "A:" << std::endl;
  printMatrix(reinterpret_cast<long *>(A), m, n, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,
                                 1),
                  reinterpret_cast<long *>(B), DIET_VOLATILE, DIET_INT, n, m,
                  oB);
  std::cout << "B (before call):" << std::endl;
  printMatrix(reinterpret_cast<long *>(B), n, m, (oB == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_INT, m, m, oC);

  if (!diet_call(profile)) {
    diet_matrix_get(diet_parameter(profile, 2), reinterpret_cast<long **>(&C),
                    NULL, &m, &n, &oC);
    std::cout << "B (after call):" << std::endl;
    printMatrix(reinterpret_cast<long *>(B), n, m, (oB == DIET_ROW_MAJOR));
    std::cout << "C:" << std::endl;
    printMatrix(reinterpret_cast<long *>(C), m, n, (oC == DIET_ROW_MAJOR));
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_MATRIX[2]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);


  /* Long */
  for (i = 0; i < SIZE * SIZE; ++i) {
    (reinterpret_cast<long *>(B))[i] = i + SIZE;
  }
  std::cout << "#### Longs" << std::endl;
  profile = diet_profile_alloc(PB_MATRIX[3], 0, 1, 2);

  diet_matrix_set(diet_parameter(profile,
                                 0),
                  reinterpret_cast<long *>(A), DIET_VOLATILE, DIET_LONGINT, m,
                  n, oA);
  std::cout << "A:" << std::endl;
  printMatrix(reinterpret_cast<long *>(A), m, n, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,
                                 1),
                  reinterpret_cast<long *>(B), DIET_VOLATILE, DIET_LONGINT, n,
                  m, oB);
  std::cout << "B (before call):" << std::endl;
  printMatrix(reinterpret_cast<long *>(B), n, m, (oB == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_LONGINT, m, m, oC);

  if (!diet_call(profile)) {
    diet_matrix_get(diet_parameter(profile, 2), reinterpret_cast<long **>(&C),
                    NULL, &m, &n, &oC);
    std::cout << "B (after call):" << std::endl;
    printMatrix(reinterpret_cast<long *>(B), n, m, (oB == DIET_ROW_MAJOR));
    std::cout << "C:" << std::endl;
    printMatrix(reinterpret_cast<long *>(C), m, n, (oC == DIET_ROW_MAJOR));
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_MATRIX[3]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);
  free(A);
  free(B);


  /* Float */
  std::cout << "#### Floats" << std::endl;
  A = reinterpret_cast<float *>(malloc(sizeof(float) * SIZE * SIZE));
  B = reinterpret_cast<float *>(malloc(sizeof(float) * SIZE * SIZE));
  for (i = 0; i < SIZE * SIZE; ++i) {
    (reinterpret_cast<float *>(A))[i] = i * 2.0;
    (reinterpret_cast<float *>(B))[i] = (i + SIZE) * 2.0;
  }
  profile = diet_profile_alloc(PB_MATRIX[4], 0, 1, 2);

  diet_matrix_set(diet_parameter(profile,
                                 0),
                  reinterpret_cast<float *>(A), DIET_VOLATILE, DIET_FLOAT, m, n,
                  oA);
  std::cout << "A:" << std::endl;
  printMatrix(reinterpret_cast<float *>(A), m, n, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,
                                 1),
                  reinterpret_cast<float *>(B), DIET_VOLATILE, DIET_FLOAT, n, m,
                  oB);
  std::cout << "B (before call):" << std::endl;
  printMatrix(reinterpret_cast<float *>(B), n, m, (oB == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_FLOAT, m, m, oC);

  if (!diet_call(profile)) {
    diet_matrix_get(diet_parameter(profile, 2), reinterpret_cast<float **>(&C),
                    NULL, &m, &n, &oC);
    std::cout << "B (after call):" << std::endl;
    printMatrix(reinterpret_cast<float *>(B), n, m, (oB == DIET_ROW_MAJOR));
    std::cout << "C:" << std::endl;
    printMatrix(reinterpret_cast<float *>(C), m, n, (oC == DIET_ROW_MAJOR));
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_MATRIX[4]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);
  free(A);
  free(B);

  /* Double */
  std::cout << "#### Doubles" << std::endl;
  A = reinterpret_cast<double *>(malloc(sizeof(double) * SIZE * SIZE));
  B = reinterpret_cast<double *>(malloc(sizeof(double) * SIZE * SIZE));
  for (i = 0; i < SIZE * SIZE; ++i) {
    (reinterpret_cast<double *>(A))[i] = i * 2.0;
    (reinterpret_cast<double *>(B))[i] = (i + SIZE) * 2.0;
  }
  profile = diet_profile_alloc(PB_MATRIX[5], 0, 1, 2);

  diet_matrix_set(diet_parameter(profile,
                                 0),
                  reinterpret_cast<double *>(A), DIET_VOLATILE, DIET_DOUBLE, m,
                  n, oA);
  std::cout << "A:" << std::endl;
  printMatrix(reinterpret_cast<double *>(A), m, n, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,
                                 1),
                  reinterpret_cast<double *>(B), DIET_VOLATILE, DIET_DOUBLE, n,
                  m, oB);
  std::cout << "B (before call):" << std::endl;
  printMatrix(reinterpret_cast<double *>(B), n, m, (oB == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile, 2),
                  NULL, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);

  if (!diet_call(profile)) {
    diet_matrix_get(diet_parameter(profile, 2), reinterpret_cast<double **>(&C),
                    NULL, &m, &n, &oC);
    std::cout << "B (after call):" << std::endl;
    printMatrix(reinterpret_cast<double *>(B), n, m, (oB == DIET_ROW_MAJOR));
    std::cout << "C:" << std::endl;
    printMatrix(reinterpret_cast<double *>(C), m, n, (oC == DIET_ROW_MAJOR));
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(C);
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_MATRIX[5]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);
  free(A);
  free(B);

  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                            \\MATRICES                               #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;




  /**
   * String types
   */
  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                              STRING                                #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;

  char *s1;
  char *s2;
  char *s3;

  s1 = reinterpret_cast<char *>(malloc(sizeof(char) * (SIZE + 1)));
  s2 = reinterpret_cast<char *>(malloc(sizeof(char) * (SIZE + 1)));
  for (i = 0; i < SIZE; ++i) {
    s1[i] = 'a';
    s2[i] = 'b';
  }
  s1[SIZE] = '\0';
  s2[SIZE] = '\0';

  /* Characters: no choice it has to be DIET_CHAR */
  std::cout << "#### Characters" << std::endl;
  profile = diet_profile_alloc(PB_STRING[0], 0, 1, 2);

  diet_string_set(diet_parameter(profile, 0), s1, DIET_VOLATILE);
  std::cout << "s1: " << s1 << std::endl;
  diet_string_set(diet_parameter(profile, 1), s2, DIET_VOLATILE);
  std::cout << "s2 (before call): " << s2 << std::endl;
  diet_string_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE);

  if (!diet_call(profile)) {
    diet_string_get(diet_parameter(profile, 2), &s3, NULL);
    std::cout << "s2 (after call): " << s2 << std::endl;
    std::cout << "s3:" << s3 << std::endl;
    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
    free(s3);
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_STRING[0]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);
  free(s1);
  free(s2);

  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                             \\STRING                                #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;



  /**
   * String types
   */
  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                           PARAMSTRING                              #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;

  s1 = reinterpret_cast<char *>(malloc(sizeof(char) * (SIZE + 1)));
  s2 = reinterpret_cast<char *>(malloc(sizeof(char) * (SIZE + 1)));
  s3 = NULL;
  for (i = 0; i < SIZE; ++i) {
    s1[i] = 'a';
    s2[i] = 'b';
  }
  s1[SIZE] = '\0';
  s2[SIZE] = '\0';

  /* Characters: no choice it has to be DIET_CHAR */
  std::cout << "#### Characters" << std::endl;
  profile = diet_profile_alloc(PB_PSTRING[0], 0, 1, 1);

  diet_paramstring_set(diet_parameter(profile, 0), s1, DIET_VOLATILE);
  std::cout << "s1: " << s1 << std::endl;
  diet_paramstring_set(diet_parameter(profile, 1), s2, DIET_VOLATILE);
  std::cout << "s2 (before call): " << s2 << std::endl;

  if (!diet_call(profile)) {
    std::cout << "s2 (after call): " << s2 << std::endl;
    diet_free_data(diet_parameter(profile, 1));
  } else {
<<<<<<< HEAD
    std::cerr << "diet_call has returned with an error code on " <<
    PB_PSTRING[0]
              << "!" << std::endl;
    error = error | (1 << errorPos);
=======
    std::cerr << "diet_call has returned with an error code on " << PB_PSTRING[0]
	      << "!" << std::endl;
    error = error | (1<<errorPos);
>>>>>>> bcbca818c8621f535ea6e567753a00a0d7b7eec7
  }
  ++errorPos;
  diet_profile_free(profile);
  free(s1);
  free(s2);

  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                          \\PARAMSTRING                              #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;




  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                                FILE                                #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;

  size_t arg_size = 0;
  struct stat buf;
  int status = 0;

  s1 = argv[1];
  s2 = argv[2];

  /* Characters: no choice it has to be DIET_CHAR */
  std::cout << "#### Characters" << std::endl;
  profile = diet_profile_alloc(PB_FILE[0], 0, 1, 2);

  if (diet_file_set(diet_parameter(profile, 0), DIET_PERSISTENT, s1)) {
    std::cout << "Error on diet_file_set on s1" << std::endl;
  }
  std::cout << "File s1: " << s1 << std::endl;
  if (diet_file_set(diet_parameter(profile, 1), DIET_VOLATILE, s2)) {
    std::cout << "Error on diet_file_set on s2" << std::endl;
  }
  std::cout << "File s2 (before call): " << s2 << std::endl;
  if (diet_file_set(diet_parameter(profile, 2), DIET_VOLATILE, NULL)) {
    std::cout << "Error on diet_file_set on s3" << std::endl;
  }

  if (!diet_call(profile)) {
    diet_file_get(diet_parameter(profile, 2), NULL, &arg_size, &s3);
    std::cout << "s2 (after call): " << s2 << std::endl;
    std::cout << "s3:" << s3 << ", size=" << arg_size << std::endl;

    if ((status = stat(s2, &buf)) || !(buf.st_mode & S_IFREG)) {
      std::cerr << "Problem on s2 in " << PB_FILE[0] << "!" << std::endl;
      error = error | (1 << errorPos);
    }

    if ((status = stat(s3, &buf)) || !(buf.st_mode & S_IFREG)) {
      std::cerr << "Problem on s3 in " << PB_FILE[0] << "!" << std::endl;
      error = error | (1 << errorPos);
    }

    diet_free_data(diet_parameter(profile, 1));
    // diet_free_data(diet_parameter(profile, 2));
    free(s3);
  } else {
    std::cerr << "diet_call has returned with an error code on " << PB_FILE[0]
              << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);

  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                               \\FILE                                #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;



  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                             CONTAINER                              #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;


  /* Characters: no choice it has to be DIET_CHAR */
  std::cout << "#### Characters" << std::endl;
  profile = diet_profile_alloc(PB_CONTAINER[0], 0, 1, 2);

  long l3, l4;
  l1 = 1;
  l2 = 2;
  l3 = 3;
  l4 = 4;
  char *p1, *p2, *p3, *p4;

  /* IN */
  dagda_create_container(&s1);
  dagda_put_scalar(&l1, DIET_LONGINT, DIET_PERSISTENT, &p1);
  dagda_add_container_element(s1, p1, 0);
  dagda_put_scalar(&l2, DIET_LONGINT, DIET_PERSISTENT, &p2);
  dagda_add_container_element(s1, p2, 1);
  diet_use_data(diet_parameter(profile, 0), s1);

  /* INOUT */
  dagda_create_container(&s2);
  dagda_put_scalar(&l3, DIET_LONGINT, DIET_PERSISTENT, &p3);
  dagda_add_container_element(s2, p3, 0);
  dagda_put_scalar(&l4, DIET_LONGINT, DIET_PERSISTENT, &p4);
  dagda_add_container_element(s2, p4, 1);
  diet_use_data(diet_parameter(profile, 1), s2);

  /* OUT */
  diet_container_set(diet_parameter(profile, 2), DIET_PERSISTENT);

  if (!diet_call(profile)) {
    diet_container_t content1;
    long *pl1, *pl2;

    s1 = (profile->parameters[1]).desc.id;
    dagda_get_container(s1);
    dagda_get_container_elements(s1, &content1);
    dagda_get_scalar(content1.elt_ids[0], &pl1, NULL);
    dagda_get_scalar(content1.elt_ids[1], &pl2, NULL);
    std::cout << "Container 2: " << std::endl
              << " - l1 = " << *pl1 << std::endl
              << " - l2 = " << *pl2 << std::endl;
    free(content1.elt_ids);


    s1 = (profile->parameters[2]).desc.id;
    dagda_get_container(s1);
    dagda_get_container_elements(s1, &content1);
    dagda_get_scalar(content1.elt_ids[0], &pl1, NULL);
    dagda_get_scalar(content1.elt_ids[1], &pl2, NULL);
    std::cout << "Container 3: " << std::endl
              << " - l1 = " << *pl1 << std::endl
              << " - l2 = " << *pl2 << std::endl;
    free(content1.elt_ids);

    diet_free_data(diet_parameter(profile, 1));
    diet_free_data(diet_parameter(profile, 2));
  } else {
    std::cerr << "diet_call has returned with an error code on " <<
    PB_CONTAINER[0] << "!" << std::endl;
    error = error | (1 << errorPos);
  }
  ++errorPos;
  diet_profile_free(profile);

  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                            \\CONTAINER                              #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;



  /* End DIET */
  diet_finalize();


  /* Check on errors */
  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                             ERRORS                                 #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout << "Error code: " << error << std::endl;
  for (i = 0; i < NB_PB; ++i) {
    if (error & (1 << i)) {
      std::cout << "## Scalars: error in problem " << PB[i] << std::endl;
    }
  }
  for (i = 0; i < NB_PB_VECTOR; ++i) {
    if (error & (1 << (i + NB_PB))) {
      std::cout << "## Vector: error in problem " << PB_VECTOR[i] << std::endl;
    }
  }
  for (i = 0; i < NB_PB_MATRIX; ++i) {
    if (error & (1 << (i + NB_PB + NB_PB_VECTOR))) {
      std::cout << "## Matrix: error in problem " << PB_MATRIX[i] << std::endl;
    }
  }
  for (i = 0; i < NB_PB_STRING; ++i) {
    if (error & (1 << (i + NB_PB + NB_PB_VECTOR + NB_PB_MATRIX))) {
      std::cout << "## String: error in problem " << PB_STRING[i] << std::endl;
    }
  }
  for (i = 0; i < NB_PB_PSTRING; ++i) {
    if (error & (1 << (i + NB_PB + NB_PB_VECTOR + NB_PB_MATRIX
                       + NB_PB_STRING))) {
      std::cout << "## Paramstring: error in problem " << PB_PSTRING[i] <<
      std::endl;
    }
  }
  for (i = 0; i < NB_PB_FILE; ++i) {
    if (error & (1 << (i + NB_PB + NB_PB_VECTOR + NB_PB_MATRIX
                       + NB_PB_STRING + NB_PB_PSTRING))) {
      std::cout << "## File: error in problem " << PB_FILE[i] << std::endl;
    }
  }
  for (i = 0; i < NB_PB_CONTAINER; ++i) {
    if (error & (1 << (i + NB_PB + NB_PB_VECTOR + NB_PB_MATRIX
                       + NB_PB_STRING + NB_PB_PSTRING + NB_PB_FILE))) {
      std::cout << "## Container: error in problem " << PB_CONTAINER[i] <<
      std::endl;
    }
  }
  std::cout <<
  "######################################################################" <<
  std::endl;
  std::cout <<
  "#                             \\ERRORS                                #" <<
  std::endl;
  std::cout <<
  "######################################################################" <<
  std::endl;

  return error;
} // main
