/****************************************************************************/
/* dmat_manips example: a DIET client for transpose, MatSUM and MatPROD     */
/*   problems (MatSUM is expanded to SqMatSUM and SqMatSUM_bis) using the   */
/*   concurrent asynchronous calls.                                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Christophe PERA           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2003/09/25 09:52:29  cpera
 * Fix bugs linked to GridRPC changes and modify log messages.
 *
 * Revision 1.3  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.2  2003/06/30 11:15:12  cpera
 * Fix bugs in ReaderWriter and new internal debug macros.
 *
 * Revision 1.1  2003/06/16 17:12:49  pcombes
 * Move the examples using the asynchronous API into this directory.
 *
 ****************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include "omnithread.h"

#include <iostream>
#include <stdlib.h>
#include <list>

#include "Global_macros.hh"

#include <time.h>

using namespace std;

#include "DIET_client.h"
static omni_mutex IOWriterLock;
#define print_matrix(item, string, reqID, mat, m, n, rm)        \
{     \
    printf("Item %d : %s", item, string); \
    printf("Matrix linked to Thread -%d- and requestID -%s-:\n",omni_thread::self()->id(), reqID); \
    size_t i, j;                           \
    printf("%s (%s-major) = \n", #mat,     \
        (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
          printf("%3f ", (mat)[j + i*(n)]);\
        else                               \
          printf("%3f ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
}

static omni_mutex mutexWorker;
static omni_mutex endy;
static omni_condition end(&endy);
//typedef diet_profile_t* threadArguments;
static size_t thread_counter = 0;

#define NB_PB 5
static const char* PB[NB_PB] =
{"T", "MatPROD", "MatSUM", "SqMatSUM", "SqMatSUM_opt"};

static size_t n_loops = 5;
static size_t n_threads = 5;
static char* path = NULL;

static int pb[NB_PB] = {0, 0, 0, 0, 0};


  void
displayArg(FILE* f, const diet_data_desc_t* arg)
{
  switch((int) arg->generic.type) {
    case DIET_SCALAR: fprintf(f, "scalar");                break;
    case DIET_VECTOR: fprintf(f, "vector (%ld)",
                          (long)arg->specific.vect.size);    break;
    case DIET_MATRIX: fprintf(f, "matrix (%ldx%ld)",
                          (long)arg->specific.mat.nb_r,
                          (long)arg->specific.mat.nb_c);   break;
    case DIET_STRING: fprintf(f, "string (%ld)",
                          (long)arg->specific.str.length); break;
    case DIET_FILE:   fprintf(f, "file (%ld)",
                          (long)arg->specific.file.size);  break;
  }
  if ((arg->generic.type != DIET_STRING)
      && (arg->generic.type != DIET_FILE)) {
    fprintf(f, " of ");
    switch ((int) arg->generic.base_type) {
      case DIET_CHAR:     fprintf(f, "char");           break;
      case DIET_BYTE:     fprintf(f, "byte");           break;
      case DIET_INT:      fprintf(f, "int");            break;
      case DIET_LONGINT:  fprintf(f, "long int");       break;
      case DIET_FLOAT:    fprintf(f, "float");          break;
      case DIET_DOUBLE:   fprintf(f, "double");         break;
      case DIET_SCOMPLEX: fprintf(f, "float complex");  break;
      case DIET_DCOMPLEX: fprintf(f, "double complex"); break;
    }
  }
  fprintf(f, "id=|%s|", arg->id);
}



  void
displayProfile(const diet_profile_t* profile, const char* path)
{
  int i = 0;
  FILE* f = stdout;
  fprintf(f, " - Service %s", path);
  for (i = 0; i <= profile->last_out; i++) {
    fprintf(f, "\n     %s ",
        (i <= profile->last_in) ? "IN   "
        : (i <= profile->last_inout) ? "INOUT"
        : "OUT  ");
    displayArg(f, &(profile->parameters[i].desc));
  }
  fprintf(f, "\n");
}

/* argv[1]: client config file path
   argv[2]: one of the strings above */
  void
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s [--poolThreadNbr <n>] [--repeat <n>] <file.cfg> [%s|%s|%s|%s|%s]\n",
      cmd, PB[0], PB[1], PB[2], PB[3], PB[4]);
  fprintf(stderr, "    ex: %s client.cfg T\n", cmd);
  fprintf(stderr, "        %s --poolThreadNbr 10 --repeat 1000 client.cfg MatSUM\n", cmd);
  exit(1);
}



class worker : public omni_thread {
  int rv;

  // Each thread of the pool do 5 call_async and wait on all ..
  // (diet_waitAnd)
  void run(void* arg) {
    size_t m, n;
    double* A = NULL;
    double* B = NULL;
    double* C = NULL;
    diet_profile_t* profile[5];
    double mat1[9] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0};
    double mat2[9] = {10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0};
    diet_matrix_order_t oA, oB, oC;
    int i;
    diet_reqID_t rst[5] = {0,0,0,0,0};
    char * requestID = new char[10];
    mutexWorker.lock();
    while ( n_loops > 0){
      n_loops-=5;
      mutexWorker.unlock();
      A = mat1;
      B = mat2;
      m = 3;
      n = 2;
      for (i = 0; i < NB_PB; i++) {
        if ((pb[i] = !strcmp(path, PB[i]))) break;
      }
      for (i = 0; i < 5; i++){
        // Square matrix problems:
        if (pb[3] || pb[4]) n = m;
        oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
        oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
        oC = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
        if (pb[0]) {
          profile[i] = diet_profile_alloc(path, -1, 0, 0);
          diet_matrix_set(diet_parameter(profile[i],0),
              A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
          //print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
        }
        else if (pb[1] || pb[2] || pb[3]) {
          profile[i] = diet_profile_alloc(path, 1, 1, 2);
          diet_matrix_set(diet_parameter(profile[i],0),
              A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
          //print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
          if (pb[1]) {
            diet_matrix_set(diet_parameter(profile[i],1),
                B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
            //print_matrix(B, n, m, (oB == DIET_ROW_MAJOR));
            diet_matrix_set(diet_parameter(profile[i],2),
                NULL, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);
          }
          else {
            diet_matrix_set(diet_parameter(profile[i],1),
                B, DIET_VOLATILE, DIET_DOUBLE, m, n, oB);
            //print_matrix(B, m, n, (oB == DIET_ROW_MAJOR));
            diet_matrix_set(diet_parameter(profile[i],2),
                NULL, DIET_VOLATILE, DIET_DOUBLE, m, n, oC);
          }
        }
        else if (pb[4]) {
          profile[i] = diet_profile_alloc(path, 0, 1, 1);
          diet_matrix_set(diet_parameter(profile[i],0),
              A, DIET_VOLATILE, DIET_DOUBLE, m, m, oA);
          //print_matrix(A, m, m, (oA == DIET_ROW_MAJOR));
          diet_matrix_set(diet_parameter(profile[i],1),
              B, DIET_VOLATILE, DIET_DOUBLE, m, m, oB);
          //print_matrix(B, m, m, (oB == DIET_ROW_MAJOR));

        }
        else {
          fprintf(stderr, "Unknown problem: %s !\n", path);
          rv = -1;
          return;
        }
        int rst_call = 0;
        if ((rst_call = diet_call_async(profile[i], &rst[i])) != 0)  printf("Error in diet_call_async return -%d-\n", rst_call);
        printf("request ID value = -%d- \n", rst[i]);
        if (rst[i] < 0) {
          printf("error in request value ID");
          return;
        }
      }
      // print input data
      IOWriterLock.lock();
      printf("***********************************************************\n");
      printf("Input data for requestID");
      for (i = 0; i < 5; i++) printf(" %d ", rst[i]);
      printf(" and omnithreadID %d \n", omni_thread::self()->id());
      for (i = 0; i < 5; i++){
        sprintf(requestID, "%d", rst[i]);
        if (pb[0]) {
          diet_matrix_get(diet_parameter(profile[i],0), NULL, NULL, (size_t*)&m, (size_t*)&n, &oA);
          print_matrix(i, "-Input data-\n",requestID, A, m, n, (oA == DIET_ROW_MAJOR));
        } 
        else if (pb[1] || pb[2] || pb[3]) {  
          diet_matrix_get(diet_parameter(profile[i],0), NULL, NULL, (size_t*)&m, (size_t*)&n, &oA);
          print_matrix(i, "-Input data-\n",requestID, A, m, n, (oA == DIET_ROW_MAJOR));
          if (pb[1]) {
            diet_matrix_get(diet_parameter(profile[i],0), NULL, NULL, (size_t*)&m, (size_t*)&n, &oB);
            print_matrix(i, "-Input data-\n",requestID, B, n, m, (oB == DIET_ROW_MAJOR));
          } 
          else {
            diet_matrix_get(diet_parameter(profile[i],0), NULL, NULL, (size_t*)&m, (size_t*)&n, &oB);
            print_matrix(i, "-Input data-\n",requestID, B, m, n, (oB == DIET_ROW_MAJOR));
          }
        } 
        else if (pb[4]) {
          diet_matrix_get(diet_parameter(profile[i],0), NULL, NULL, (size_t*)&m, (size_t*)&n, &oA);
          diet_matrix_get(diet_parameter(profile[i],0), NULL, NULL, (size_t*)&m, (size_t*)&n, &oB);
          print_matrix(i, "-Input data-\n",requestID, A, m, m, (oA == DIET_ROW_MAJOR));
          print_matrix(i, "-Input data-\n",requestID,B, m, m, (oB == DIET_ROW_MAJOR));
        }
      }
      printf("***********************************************************\n");
      IOWriterLock.unlock();
      // test all return rst
      int rst_call = 0;
      if ((rst_call = diet_wait_and((diet_reqID_t*)&rst, (unsigned int)5)) != 0) printf("Error in diet_wait_and\n");
      //mutexWorker.lock();
      IOWriterLock.lock();
      printf("***********************************************************\n");
      printf("Result data for requestID");
      for (i = 0; i < 5; i++) printf(" %d ", rst[i]);
      printf(" and omnithreadID %d \n", omni_thread::self()->id());
      for (i = 0; i < 5; i++){
        sprintf(requestID, "%d", rst[i]);
        if (pb[0]) {
          diet_matrix_get(diet_parameter(profile[i],0), NULL, NULL, (size_t*)&m, (size_t*)&n, &oA);
          print_matrix(i, "-result-\n", requestID, A, m, n, (oA == DIET_ROW_MAJOR));
        }
        else if (pb[4]) {
          diet_matrix_get(diet_parameter(profile[i],0), NULL, NULL, (size_t*)&m, (size_t*)&n, &oB);
          print_matrix(i, "-result-\n", requestID, B, m, n, (oB == DIET_ROW_MAJOR));
        }
        else {
          diet_matrix_get(diet_parameter(profile[i],2), &C, NULL, (size_t*)&m, (size_t*)&n, &oC);
          print_matrix(i, "-result-\n", requestID, C, m, n, (oC == DIET_ROW_MAJOR));
          diet_free_data(diet_parameter(profile[i],2));
        }
      }
      printf("***********************************************************\n");
      IOWriterLock.unlock();
      for (i = 0; i < 5; i++){
        diet_cancel(rst[i]);
        diet_profile_free(profile[i]);
      }
      mutexWorker.lock();
    }
    delete requestID;
    mutexWorker.unlock();
    return;
  }

  // the destructor of a class that inherits from omni_thread should never be
  // public (otherwise the thread object can be destroyed while the
  // underlying thread is still running).
  ~worker() {
      IOWriterLock.lock();
    printf("Destroy thread");
      IOWriterLock.unlock();
    mutexWorker.lock();
    if (thread_counter < (n_threads-1)){
      thread_counter++;
    }
    else {
      end.broadcast();
    }
    mutexWorker.unlock();
  }

  void* make_arg(int i) { return (void*)new int(i); }

  public:

  worker(int id) : omni_thread(make_arg(id)) {
    rv = id;
    start();
  }
};


  int
main(int argc, char* argv[])
{
  int i;
  printf("Asynchronous client Type 3 (diet_wait_and)\n");
  srand(time(NULL));
  for (i = 1; i < argc - 2; i++) {
    if (strcmp("--repeat", argv[i]) == 0) {
      n_loops = 5*(atoi(argv[i + 1]));
      i++;
      memcpy(argv + i - 2, argv + i, (argc - i)*sizeof(char*));
      i -= 2;
      argc -= 2;
    }
    else if (strcmp("--poolThreadNbr", argv[i]) == 0){
      n_threads =  atoi(argv[i + 1]);
      i++;
      memcpy(argv + i - 2, argv + i, (argc - i)*sizeof(char*));
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
  path = argv[argc - 1];
  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }
  // create pool thread and give job ....
  int incr = 0;
  worker* tab[n_threads];
  while (incr < (int)n_threads){
    tab[incr] = new worker(incr);
    incr++;
  }
  //omni_thread::sleep(5);
  end.wait();
  diet_finalize();
  printf("END of asynchronous client Type 3 (diet_wait_and)\n");
  return 0;
}
