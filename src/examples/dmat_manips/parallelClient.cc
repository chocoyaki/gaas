/**
 * @file parallelClient.cc
 *
 * @brief  a DIET client for using the concurrent asynchronous calls.
 *
 * @author  Christophe PERA (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


/* #include <getopt.h> */
#include <iostream>
using namespace std;
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef __WIN32__
#include <unistd.h>
#else
#include <Winsock2.h>
#include <windows.h>
#endif
#include <string.h>

#include "Global_macros.hh"
#include "omnithread.h"
#include "DIET_client.h"


#define print_matrix(string, reqID, mat, m, n, rm)                      \
  {                                                                     \
    IO_WRITER_LOCK.lock();                                              \
    printf("-------------------\n");                                    \
    printf(string);                                                     \
    printf("Matrix linked to Thread -%d- and requestID -%s-:\n", \
           omni_thread::self()->id(), reqID); \
    size_t i, j;                                                        \
    printf("%s (%s-major) = \n", # mat,                                  \
           (rm) ? "row" : "column");                                    \
    for (i = 0; i < (m); i++) {                                         \
      for (j = 0; j < (n); j++) {                                       \
        if (rm) {                                                         \
          printf("%3f ", (mat)[j + i * (n)]); }                             \
        else {                                                            \
          printf("%3f ", (mat)[i + j * (m)]); }                             \
      }                                                                 \
      printf("\n");                                                     \
    }                                                                   \
    printf("-------------------\n");                                    \
    printf("\n");                                                       \
    IO_WRITER_LOCK.unlock();                                            \
  }

static omni_mutex IO_WRITER_LOCK;
static omni_mutex MUTEX_WORKER;
static omni_mutex ENDY;
static omni_condition END(&ENDY);
// typedef diet_profile_t* threadArguments;
static size_t thread_counter = 0;

#define NB_PB 5
static const char *PB[NB_PB] =
{"T", "MatPROD", "MatSUM", "SqMatSUM", "SqMatSUM_opt"};

static size_t n_loops = 5;
static size_t n_threads = 5;
static char *path = NULL;

static int IS_PB[NB_PB] = {0, 0, 0, 0, 0};


/* argv[1]: client config file path
   argv[2]: one of the strings above */
void
usage(char *cmd) {
  fprintf(
    stderr,
    "Usage: %s [--poolThreadNbr <n>] [--repeat <n>] <file.cfg> [%s|%s|%s|%s|%s]\n",
    cmd, PB[0], PB[1], PB[2], PB[3], PB[4]);
  fprintf(stderr, "    ex: %s client.cfg T\n", cmd);
  fprintf(stderr,
          "        %s --poolThreadNbr 10 --repeat 1000 client.cfg MatSUM\n",
          cmd);
  exit(1);
}



class worker: public omni_thread {
int rv;
void
run(void *arg) {
  size_t m, n;
  double *A = NULL;
  double *B = NULL;
  double *C = NULL;
  diet_profile_t *profile = NULL;
  double mat1[9] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  double mat2[9] = {10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0};
  diet_matrix_order_t oA, oB, oC;
  int i;
  char *requestID = new char[10];
  MUTEX_WORKER.lock();
  while (n_loops > 0) {
    printf("---------------- %u loops left -------------------\n",
           (unsigned int) n_loops);
    n_loops--;
    MUTEX_WORKER.unlock();
    A = mat1;
    B = mat2;
    m = 3;
    n = 2;
    for (i = 0; i < NB_PB; i++) {
      if ((IS_PB[i] = !strcmp(path, PB[i]))) {
        break;
      }
    }
    // Square matrix problems:
    if (IS_PB[3] || IS_PB[4]) {
      n = m;
    }
    oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oC = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    if (IS_PB[0]) {
      profile = diet_profile_alloc(path, -1, 0, 0);
      diet_matrix_set(diet_parameter(profile, 0),
                      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
    } else if (IS_PB[1] || IS_PB[2] || IS_PB[3]) {
      profile = diet_profile_alloc(path, 1, 1, 2);
      diet_matrix_set(diet_parameter(profile, 0),
                      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      if (IS_PB[1]) {
        diet_matrix_set(diet_parameter(profile, 1),
                        B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
        diet_matrix_set(diet_parameter(profile, 2),
                        NULL, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);
      } else {
        diet_matrix_set(diet_parameter(profile, 1),
                        B, DIET_VOLATILE, DIET_DOUBLE, m, n, oB);
        diet_matrix_set(diet_parameter(profile, 2),
                        NULL, DIET_VOLATILE, DIET_DOUBLE, m, n, oC);
      }
    } else if (IS_PB[4]) {
      profile = diet_profile_alloc(path, 0, 1, 1);
      diet_matrix_set(diet_parameter(profile, 0),
                      A, DIET_VOLATILE, DIET_DOUBLE, m, m, oA);
      diet_matrix_set(diet_parameter(profile, 1),
                      B, DIET_VOLATILE, DIET_DOUBLE, m, m, oB);
    } else {
      fprintf(stderr, "Unknown problem: %s!\n", path);
      rv = -1;
      delete [] requestID;
      return;
    }
    diet_reqID_t rst;
    if (diet_call_async(profile, &rst) != 0) {
      printf("error in diet_call_async ...\n");
    }
    printf("Received request ID = -%d- \n", rst);
    if (rst >= 0) {
      // print input data
      sprintf(requestID, "%d", rst);
      if (IS_PB[0]) {
        print_matrix("-Input data-\n", requestID, A, m, n,
                     (oA == DIET_ROW_MAJOR));
      } else if (IS_PB[1] || IS_PB[2] || IS_PB[3]) {
        print_matrix("-Input data-\n", requestID, A, m, n,
                     (oA == DIET_ROW_MAJOR));
        if (IS_PB[1]) {
          print_matrix("-Input data-\n", requestID, B, n, m,
                       (oB == DIET_ROW_MAJOR));
        } else {
          print_matrix("-Input data-\n", requestID, B, m, n,
                       (oB == DIET_ROW_MAJOR));
        }
      } else if (IS_PB[4]) {
        print_matrix("-Input data-\n", requestID, A, m, m,
                     (oA == DIET_ROW_MAJOR));
        print_matrix("-Input data-\n", requestID, B, m, m,
                     (oB == DIET_ROW_MAJOR));
      }
      printf("Calling diet_wait ...\n");
      diet_wait(rst);

      // MUTEX_WORKER.lock();
      if (IS_PB[0]) {
        diet_matrix_get(diet_parameter(profile,
                                       0), NULL, NULL, (size_t *) &m,
                        (size_t *) &n, &oA);
        print_matrix("-result-\n", requestID, A, m, n, (oA == DIET_ROW_MAJOR));
      } else if (IS_PB[4]) {
        diet_matrix_get(diet_parameter(profile,
                                       0), NULL, NULL, (size_t *) &m,
                        (size_t *) &n, &oB);
        print_matrix("-result-\n", requestID, B, m, n, (oB == DIET_ROW_MAJOR));
      } else {
        diet_matrix_get(diet_parameter(profile,
                                       2), &C, NULL, (size_t *) &m,
                        (size_t *) &n, &oC);
        print_matrix("-result-\n", requestID, C, m, n, (oC == DIET_ROW_MAJOR));
        // diet_free_data(diet_parameter(profile, 2));
      }
      diet_cancel(rst);
    } else {
      printf("error in diet_call_async ...\n");
      MUTEX_WORKER.unlock();
      delete [] requestID;
      return;
    }
    diet_profile_free(profile);
    MUTEX_WORKER.lock();
  }
  MUTEX_WORKER.unlock();
  delete [] requestID;
  return;
} // run

// the destructor of a class that inherits from omni_thread should never be
// public (otherwise the thread object can be destroyed while the
// underlying thread is still running).
~worker() {
  printf("Destroying thread...\n");
  MUTEX_WORKER.lock();
  if (thread_counter < (n_threads - 1)) {
    thread_counter++;
  } else {
    END.broadcast();
  }
  MUTEX_WORKER.unlock();
}

void *
make_arg(int i) {
  return (void *) new int(i);
}

public:

worker(int id): omni_thread(make_arg(id)) {
  rv = id;
  start();
}
};


int
main(int argc, char *argv[]) {
  int i;
  srand(time(NULL));
  for (i = 1; i < argc - 2; i++) {
    if (strcmp("--repeat", argv[i]) == 0) {
      n_loops = atoi(argv[i + 1]);
      i++;
      memcpy(argv + i - 2, argv + i, (argc - i) * sizeof(char *));
      i -= 2;
      argc -= 2;
    } else if (strcmp("--poolThreadNbr", argv[i]) == 0) {
      n_threads = atoi(argv[i + 1]);
      i++;
      memcpy(argv + i - 2, argv + i, (argc - i) * sizeof(char *));
      i -= 2;
      argc -= 2;
    } else {
      fprintf(stderr, "Unrecognized option %s\n", argv[i]);
      usage(argv[0]);
    }
  }

  if (argc - i != 2) {
    usage(argv[0]);
  }

  printf("\nBeginning of asynchronous client Type 1 (diet_wait)\n");

  path = argv[argc - 1];
  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed!\n");
    return 1;
  }
  // create pool thread and give job ....
  int incr = 0;
  worker** tab = new worker*[n_threads];
  while (incr < (int) n_threads) {
    tab[incr] = new worker(incr);
    incr++;
  }
  // omni_thread::sleep(5);
  END.wait();
  diet_finalize();
  printf("End of asynchronous client Type 1 (diet_wait)\n");
  return 0;
} // main
