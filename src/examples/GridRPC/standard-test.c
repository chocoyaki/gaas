/****************************************************************************/
/* Interoperability test code of the GridRPC End-User API                   */
/*  Author(s):                                                              */
/*    - Yusuke Tanimura (yusuke.tanimura@aist.go.jp)                        */
/*                                                                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2006/07/21 09:29:22  eboix
 *  - Added the first brick for test suite [disabled right now].
 *  - Doxygen related cosmetic changes.  --- Injay2461
 *
 * Revision 1.2  2006/06/29 12:29:00  ecaron
 * Correct header
 *
 * Revision 1.1  2006/06/29 12:23:26  ecaron
 * Add client for GridRPC interoperability checking
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <grpc.h>
#include <DIET_client.h>

#define NCALLS 3

void initialize_test_1(char *);
void initialize_test_2(char *);
void finalize_test_1(char *);
void finalize_test_2();
void mgmt_func_handle_test_1(char *, char **);
void mgmt_func_handle_test_2(char **);
void mgmt_func_handle_test_3(char *, char *, char **);
void mgmt_func_handle_test_4(char *, char *, char **);
void mgmt_func_handle_test_5(char *, char **);
void mgmt_func_handle_test_6();
void mgmt_func_handle_test_7(char *, char **);
void mgmt_func_handle_test_8(char *, char **);
void mgmt_func_handle_test_9();
void call_test_1(char *, char **);
void call_test_2();
void call_test_3(char *, char **);
void call_test_4();
void wait_test_1(char *, char **);
void wait_test_2(char *);
void wait_test_3();
void wait_test_4(char *, char **);
void wait_test_5(char *);
void wait_test_6();
void wait_test_7(char *, char **);
void wait_test_8(char *);
void wait_test_9();
void wait_test_10(char *, char **);
void wait_test_11();
void wait_test_12(char *, char **);
void wait_test_13();
void probe_test_1(char *, char **);
void probe_test_2(char *, char **);
void probe_test_3(char *);
void probe_test_4();
void probe_test_5(char *, char **);
void probe_test_6(char *, char **);
void probe_test_7(char *);
void probe_test_8();
void cancel_test_1(char *, char **);
void cancel_test_2(char *);
void cancel_test_3();
void cancel_test_4(char *, char **);
void cancel_test_5();
void error_report_test_1(char *);
void error_report_test_2(char *);
void error_report_test_3(char *, char **);
void error_report_test_4(char *);
void error_report_test_5(char *, char **);
void error_report_test_6(char *, char **);
void error_report_test_7(char *, char **);


int main(int argc, char **argv)
{
  if (argc != 7){
    fprintf(stderr,
            "Usage: %s <config> <server> <add_func> <sleep_func> <exit_func> <loop_func>\n",
            argv[0]);
    exit(-1);
  } 

  /* Initialize Test */
  initialize_test_1(argv[1]);
  initialize_test_2(argv[1]);

  /* Finalize Test */
  finalize_test_1(argv[1]);
  finalize_test_2();

  /* Management Function Handle Test */
  mgmt_func_handle_test_1(argv[1], &argv[3]);
  mgmt_func_handle_test_2(&argv[3]);
  mgmt_func_handle_test_3(argv[1], argv[2], &argv[3]);
  mgmt_func_handle_test_4(argv[1], argv[2], &argv[3]);
  mgmt_func_handle_test_5(argv[1], &argv[3]);
  mgmt_func_handle_test_6();
  mgmt_func_handle_test_7(argv[1], &argv[3]);
  mgmt_func_handle_test_8(argv[1], &argv[3]);
  mgmt_func_handle_test_9();

  /* Call Test */
  call_test_1(argv[1], &argv[3]);
  call_test_2();
  call_test_3(argv[1], &argv[3]);
  call_test_4();

  /* Wait Test */
  wait_test_1(argv[1], &argv[3]);
  wait_test_2(argv[1]);
  wait_test_3();
  wait_test_4(argv[1], &argv[3]);
  wait_test_5(argv[1]);
  wait_test_6();
  wait_test_7(argv[1], &argv[3]);
  wait_test_8(argv[1]);
  wait_test_9();
  wait_test_10(argv[1], &argv[3]);
  wait_test_11();
  wait_test_12(argv[1], &argv[3]);
  wait_test_13();

  /* Probe Test */
  probe_test_1(argv[1], &argv[3]);
  probe_test_2(argv[1], &argv[3]);
  probe_test_3(argv[1]);
  probe_test_4();
  probe_test_5(argv[1], &argv[3]);
  probe_test_6(argv[1], &argv[3]);
  probe_test_7(argv[1]);
  probe_test_8();

  /* Cancel Test */
  cancel_test_1(argv[1], &argv[3]);
  cancel_test_2(argv[1]);
  cancel_test_3();
  cancel_test_4(argv[1], &argv[3]);
  cancel_test_5();

  /* Error Reporting Test */
  error_report_test_1(argv[1]);
  error_report_test_2(argv[1]);
  error_report_test_3(argv[1], &argv[3]);
  error_report_test_4(argv[1]);
  error_report_test_5(argv[1], &argv[3]);
  error_report_test_6(argv[1], &argv[3]);
  error_report_test_7(argv[1], &argv[3]);

  return 0;
}


/*
 * Call grpc_initialize() with a correct configuration file,
 * checking GRPC_NO_ERROR returned.
 */
void initialize_test_1(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Initialize Test 1: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  } 
}


/*
 * Call grpc_initialize() with a correct configuration file twice,
 * checking GRPC_ALREADY_INITIALIZED returned.
 */
void initialize_test_2(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Initialize Test 2: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_initialize(config_file);
  if (err != GRPC_ALREADY_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_finalize() in right way,
 * checking GRPC_NO_ERROR returned.
 */
void finalize_test_1(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Finalize Test 1: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    printf("Failure: (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_finalize() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void finalize_test_2()
{
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Finalize Test 2: ");

  err = grpc_finalize();
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_function_handle_default() with an available function name,
 * checking GRPC_NO_ERROR returned with a pointer of the initialized
 * function handle.
 */
void mgmt_func_handle_test_1(char *config_file, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Management Function Handle Test 1: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");
  
  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_function_handle_default() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void mgmt_func_handle_test_2(char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Management Function Handle Test 2: ");

  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_function_handle_init() with an available set of
 * the function name and the server name, checking GRPC_NO_ERROR
 * returned with a pointer of the initialized function handle.
 */ 
void mgmt_func_handle_test_3(char *config_file, char *server, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Management Function Handle Test 3: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_function_handle_init(&handle, server, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_function_handle_init() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void mgmt_func_handle_test_4(char *config_file, char *server, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Management Function Handle Test 4: ");

  err = grpc_function_handle_init(&handle, server, func_list[0]);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_function_handle_destruct() in right way,
 * checking GRPC_NO_ERROR returned.
 */
void mgmt_func_handle_test_5(char *config_file, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Management Function Handle Test 5: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_function_handle_destruct() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void mgmt_func_handle_test_6()
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Management Function Handle Test 6: ");

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_get_handle() with a valid session ID, checking
 * GRPC_NO_ERROR returned with a pointer of the function
 * handle specified by ID.
 */
void mgmt_func_handle_test_7(char *config_file, char **func_list)
{
  grpc_function_handle_t handle, *handle_tmp;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;
  printf("Management Function Handle Test 7: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  err = grpc_call_async(&handle, &id, x, &y);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err)); 
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }

  err = grpc_get_handle(&handle_tmp, id);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  err = grpc_wait(id);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait() failed: %s\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  err = grpc_call_async(handle_tmp, &id, x, &y);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s, and the handle cannot be used.)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  err = grpc_wait(id);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s, and the handle cannot be used.)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_get_handle() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
void mgmt_func_handle_test_8(char *config_file, char **func_list)
{
  grpc_function_handle_t handle, *handle_tmp;
  grpc_error_t err;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Management Function Handle Test 8: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }

  err = grpc_get_handle(&handle_tmp, id);
  if (err != GRPC_INVALID_SESSION_ID) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_get_handle() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returnd.
 */
void mgmt_func_handle_test_9()
{
  grpc_function_handle_t *handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Management Function Handle Test 9: ");

  err = grpc_get_handle(&handle, id);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_call() with an initialized handle and valid arguments,
 * checking GRPC_NO_ERROR returned with correct output arguments.
 */
void call_test_1(char *config_file, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  int x = 3, y = 0;
  printf("Call Test 1: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }

  err = grpc_call(&handle, x, &y);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  if (y != (x + 1)) {
    printf("Failure (wrong result: y = %d)\n", y); 
  } else {
    printf("Success\n");
  }

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  } 
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_call() before calling grpc_initialize(), checking
 * GRPC_NOT_INITIALIZED returned.
 */
void call_test_2()
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  int x = 3, y = 0;
  printf("Call Test 2: ");

  err = grpc_call(&handle, x, &y);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}
 

/*
 * Call grpc_call_async() with an initialized handle and valid
 * arguments, checking GRPC_NO_ERROR returned with a valid
 * session ID.
 */
void call_test_3(char *config_file, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;
  printf("Call Test 3: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }

  err = grpc_call_async(&handle, &id, x, &y);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  err = grpc_wait(id);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait() failed: %s\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  if (y != (x + 1)) {
    printf("Failure (wrong result: y = %d)\n", y);
  } else {
    printf("Success\n");
  }

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/* 
 * Call grpc_call_async() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void call_test_4()
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;
  printf("Call Test 4: ");

  err = grpc_call_async(&handle, &id, x, &y);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_wait() with a valid session ID to be synchronized,
 * checking GRPC_NO_ERROR returned.
 */
void wait_test_1(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i, j, x = 3, y = 0;
  printf("Wait Test 1: ");

  for (i = 0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  for (i = 0; i<NCALLS; i++) {
    err = grpc_wait(id[i]);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
    if (y != (x + 1)) {
      printf("Failure (wrong result: y = %d)\n", y);
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  } 
  printf("Success\n");

  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_wait() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */ 
void wait_test_2(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Wait Test 2: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_wait(id);
  if (err != GRPC_INVALID_SESSION_ID) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_wait() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void wait_test_3()
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Wait Test 3: ");

  err = grpc_wait(id);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_wait_and() with an array of the valid session IDs,
 * checking GRPC_NO_ERROR returned.
 */
void wait_test_4(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i, j, x = 3, y = 0;
  printf("Wait Test 4: ");

  for (i = 0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_wait_and(id, NCALLS);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
    grpc_finalize();
    return;
  }
  if (y != (x + 1)) {
    printf("Failure (wrong result: y = %d)\n", y);
    for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
    grpc_finalize();
    return;
  }
  printf("Success\n");

  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_wait_and() with a session ID array which contains
 * at least one invalid ID, checking GRPC_INVALID_SESSION_ID
 * returned.
 */
void wait_test_5(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i;
  printf("Wait Test 5: ");

  for (i = 0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_wait_and(id, NCALLS);
  if (err != GRPC_INVALID_SESSION_ID) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_wait_and() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void wait_test_6()
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Wait Test 6: ");

  err = grpc_wait_and(&id, 1);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_wait_or() with a valid session ID array,
 * checking GRPC_NO_ERROR returned with a pointer of
 * the completed session ID.
 */
void wait_test_7(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, found, x[NCALLS];
  printf("Wait Test 7: ");
  
  for (i = 0; i<NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    if (i == 0) x[i] = 1;
    else x[i] = 10;
  }

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[1]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x[i]);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  for (i = NCALLS; i>0; i--) {
    err = grpc_wait_or(id, i, &ret_id);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }

    /* Find the returned id and replace it with the last id */
    found = 0;
    for (j = 0; j<i; j++) {
      if (ret_id == id[j]) {
        id[j] = id[i-1];
        id[i-1] = ret_id;
        found = 1;
        break;
      }
    }
    if (found == 0) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  printf("Success\n");

  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_wait_or() with a session ID array which
 * contains at least one invalid ID, checking
 * GRPC_INVALID_SESSION_ID returned.
 */
void wait_test_8(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i;
  printf("Wait Test 8: ");

  for (i = 0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_wait_or(id, NCALLS, &ret_id);
  if (err != GRPC_INVALID_SESSION_ID) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_wait_or() before calling grpc_wait_or(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void wait_test_9()
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID, ret_id = GRPC_SESSIONID_VOID;
  printf("Wait Test 9: ");

  err = grpc_wait_or(&id, 1, &ret_id);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_wait_all() in right way, checking
 * GRPC_NO_ERROR returned.
 */
void wait_test_10(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i, j, x = 3, y[NCALLS];
  printf("Wait Test 10: ");

  for (i = 0; i<NCALLS; i++) y[i] = 0;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y[i]);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_wait_all();
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
    grpc_finalize();
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    if (y[i] != (x + 1)) {
      printf("Failure (wrong result: y[%d] = %d)\n", i, y[i]);
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  printf("Success\n");

  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_wait_all() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void wait_test_11()
{
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Wait Test 11: ");

  err = grpc_wait_all();
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_wait_any() in right way, checking GRPC_NO_ERROR
 * returned with an ID pointer of the session which is
 * completed.
 */
void wait_test_12(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, counter = 0, x = 3, y[NCALLS];
  printf("Wait Test 12: ");

  for (i = 0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y[i]);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  while (1) {
    err = grpc_wait_any(&ret_id);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
    counter++;
    if (counter == NCALLS) break;
  }
  for (i = 0; i<NCALLS; i++) {  
    if (y[i] != (x + 1)) {
      printf("Failure (wrong result: y[%d] = %d)\n", i, y[i]);
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  printf("Success\n");

  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_wait_any() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void wait_test_13()
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Wait Test 13: ");

  err = grpc_wait_any(&id);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_probe() with a session ID which is completed,
 * checking GRPC_NO_ERROR returned.
 */
void probe_test_1(char *config_file, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;
  printf("Probe Test 1: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  err = grpc_call_async(&handle, &id, x, &y);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }

  while (1) {
    err = grpc_probe(id);
    if (err == GRPC_NO_ERROR) {
      printf("Success\n");
      break;
    } else if (err != GRPC_NOT_COMPLETED) {
      printf("Failure: (%s)\n", grpc_error_string(err));
      grpc_function_handle_destruct(&handle);
      grpc_finalize();
      return;
    }
  }

  err = grpc_wait(id);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait() failed: %s\n", grpc_error_string(err));
  }

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_probe() with a session ID which is not completed,
 * checking GRPC_NOT_COMPLETED returned.
 */
void probe_test_2(char *config_file, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 10;
  printf("Probe Test 2: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[1]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  err = grpc_call_async(&handle, &id, x);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }

  while (1) {
    err = grpc_probe(id);
    if (err == GRPC_NOT_COMPLETED) {
      printf("Success\n");
      break;
    }else if (err == GRPC_NO_ERROR) {
      printf("Failure (The session was completed beofore calling grpc_probe().)\n");
      break;
    } else {
      printf("Failure: (%s)\n", grpc_error_string(err));
      grpc_function_handle_destruct(&handle);
      grpc_finalize();
      return;
    }
  }

  err = grpc_wait(id);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait() failed: %s\n", grpc_error_string(err));
  }

  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_probe() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
void probe_test_3(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Probe Test 3: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_probe(id);
  if (err != GRPC_INVALID_SESSION_ID) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_probe() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returnd.
 */
void probe_test_4()
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Probe Test 4: ");

  err = grpc_probe(id);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_probe_or() with a session ID array which contains
 * at least one completed session, checking GRPC_NO_ERROR
 * returned with a pointer of the probed session ID.
 */
void probe_test_5(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, x[NCALLS];
  printf("Probe Test 5: ");

  for (i = 0; i<NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    if (i == 0) x[i] = 0;
    else x[i] = 10;
  }
  
  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[1]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_probe_or(id, NCALLS, &ret_id);
  while (1) {
    if (err == GRPC_NO_ERROR) {
      printf("Success\n");
      break;
    } else if (err != GRPC_NONE_COMPLETED) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (i = 0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_wait_all();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait_all() failed: %s\n", grpc_error_string(err));
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_probe_or() with a session ID array which
 * does not contain any of completed sessions, checking
 * GRPC_NONE_COMPLETED returned with an invalid session ID.
 */ 
void probe_test_6(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, x[NCALLS];
  printf("Probe Test 6: ");

  for (i = 0; i<NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    x[i] = 10;
  }

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[1]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_probe_or(id, NCALLS, &ret_id);
  if (err == GRPC_NONE_COMPLETED) {
    if (ret_id == GRPC_SESSIONID_VOID) printf("Success\n");
    else printf("Failure (The returned ID was not GRPC_SESSIONID_VOID.)\n");
  } else if (err == GRPC_NO_ERROR) {
    printf("Failure (All sessions were completed before calling grpc_probe_or().)");
  } else {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (i = 0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }

  err = grpc_wait_all();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait_all() failed: %s\n", grpc_error_string(err));
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_probe_or() with a session ID array which contains
 * an invalid session ID, checking GRPC_INVALID_SESSION_ID
 * returned.
 */
void probe_test_7(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id;
  int i;
  printf("Probe Test 7: ");

  for (i = 0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_probe_or(id, NCALLS, &ret_id);
  if (err != GRPC_INVALID_SESSION_ID) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_probe_or() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void probe_test_8()
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID, ret_id = GRPC_SESSIONID_VOID;
  printf("Probe Test 8: ");

  err = grpc_probe_or(&id, 1, &ret_id);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_cancel() with a valid session ID, checking
 * GRPC_NO_ERROR returned after the specified session
 * is canceleda.
 */
void cancel_test_1(char *config_file, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id;
  int x = 1;
  printf("Cancel Test 1: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[3]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  err = grpc_call_async(&handle, &id, x);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }

  err = grpc_cancel(id);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_wait(id);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_cancel() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
void cancel_test_2(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Cancel Test 2: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }        

  err = grpc_cancel(id);
  if (err != GRPC_INVALID_SESSION_ID) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_cancel() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void cancel_test_3()
{
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Cancel Test 3: ");

  err = grpc_cancel(id);
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_cancel_all() in right way, checking GRPC_NO_ERROR
 * returned after all of the executing sessions are canceled.
 */
void cancel_test_4(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i, j, x = 1;
  printf("Cancel Test 4: ");

  for (i = 0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[3]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_cancel_all();
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (i = 0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_wait_all();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait_all() failed: %s\n", grpc_error_string(err));
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_cancel_all() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
void cancel_test_5()
{
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Cancel Test 5: ");

  err = grpc_cancel_all();
  if (err != GRPC_NOT_INITIALIZED) {
    printf("Failure (%s)\n", grpc_error_string(err));
    return;
  }
  printf("Success\n");
}


/*
 * Call grpc_error_string() with a defined error code,
 * checking the corresponded error string returned.
 */
void error_report_test_1(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Error Reporting Test 1:\n");
        
  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
        
  printf("  GRPC_NO_ERROR: %s\n", grpc_error_string(GRPC_NO_ERROR));
  printf("  GRPC_NOT_INITIALIZED: %s\n", grpc_error_string(GRPC_NOT_INITIALIZED)); 
  printf("  GRPC_ALREADY_INITIALIZED: %s\n", grpc_error_string(GRPC_ALREADY_INITIALIZED));
  printf("  GRPC_CONFIGFILE_NOT_FOUND: %s\n", grpc_error_string(GRPC_CONFIGFILE_NOT_FOUND));
  printf("  GRPC_CONFIGFILE_ERROR: %s\n", grpc_error_string(GRPC_CONFIGFILE_ERROR));
  printf("  GRPC_SERVER_NOT_FOUND: %s\n", grpc_error_string(GRPC_SERVER_NOT_FOUND));
  printf("  GRPC_FUNCTION_NOT_FOUND: %s\n", grpc_error_string(GRPC_FUNCTION_NOT_FOUND));
  printf("  GRPC_INVALID_FUNCTION_HANDLE: %s\n", grpc_error_string(GRPC_INVALID_FUNCTION_HANDLE));
  printf("  GRPC_INVALID_SESSION_ID: %s\n", grpc_error_string(GRPC_INVALID_SESSION_ID));
  printf("  GRPC_RPC_REFUSED: %s\n", grpc_error_string(GRPC_RPC_REFUSED));
  printf("  GRPC_COMMUNICATION_FAILED: %s\n", grpc_error_string(GRPC_COMMUNICATION_FAILED));
  printf("  GRPC_SESSION_FAILED: %s\n", grpc_error_string(GRPC_SESSION_FAILED));
  printf("  GRPC_NOT_COMPLETED: %s\n", grpc_error_string(GRPC_NOT_COMPLETED));
  printf("  GRPC_NONE_COMPLETED: %s\n", grpc_error_string(GRPC_NONE_COMPLETED));
  printf("  GRPC_OTHER_ERROR_CODE: %s\n", grpc_error_string(GRPC_OTHER_ERROR_CODE));
  printf("  GRPC_UNKNOWN_ERROR_CODE: %s\n", grpc_error_string(GRPC_UNKNOWN_ERROR_CODE));
  printf("  GRPC_LAST_ERROR_CODE: %s\n", grpc_error_string(GRPC_LAST_ERROR_CODE));

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_error_string with a non-defined error code, checking
 * the string which means "GRPC_UNKNOWN_ERROR_CODE" returned.
 */
void error_report_test_2(char *config_file)
{
  grpc_error_t err = GRPC_NO_ERROR;
  printf("Error Reporting Test 2:\n");
  
  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = -1;
  printf("  A non-defined error code (-1) corresponds to '%s'\n", grpc_error_string(err));
  err = 1000;
  printf("  A non-defined error code (1000) corresponds to '%s'\n", grpc_error_string(err));

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_get_error() with a valid session ID, checking
 * the error code associated with the given session returned.
 */
void error_report_test_3(char *config_file, char **func_list)
{
  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;
  printf("Error Reporting Test 3: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  err = grpc_function_handle_default(&handle, func_list[0]);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  err = grpc_call_async(&handle, &id, x, &y);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  
  err = grpc_get_error(id);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (Wrong error returned: %s)\n", grpc_error_string(err));
    grpc_function_handle_destruct(&handle);
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_wait(id);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_function_handle_destruct(&handle);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_get_error() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
void error_report_test_4(char *config_file)
{
  grpc_error_t err;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Error Reporting Test 4: ");
  
  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }

  err = grpc_get_error(id);
  if (err != GRPC_INVALID_SESSION_ID) {
    printf("Failure (%s)\n", grpc_error_string(err));
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_get_error_failed_sessionid() when there is
 * no failed session, checking GRPC_NO_ERROR returned.
 */
void error_report_test_5(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, x = 3, y[NCALLS];
  printf("Error Reporting Test 5: ");

  for (i = 0; i<NCALLS; i++){
    y[i] = 0;
    id[i] = GRPC_SESSIONID_VOID;
  }

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  err = grpc_wait_all();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait_all() failed: %s\n", grpc_error_string(err));
    for (i = 0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    if (y[i] != (x + 1)) {
      fprintf(stderr, "Wrong result: y[%d] = %d\n", i, y[i]);
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_get_failed_sessionid(&ret_id);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (ID %d got the error: %s)", ret_id, grpc_error_string(err));
    for (i = 0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }
  printf("Success\n");

  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_get_failed_sessionid() when there is one
 * failed session, checking a pointer of that session ID
 * returned as idPtr.
 */
void error_report_test_6(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, x = 10;
  printf("Error Reporting Test 6: ");

  for (i = 0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;
  
  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    if (i == 0) err = grpc_function_handle_default(&handle[i], func_list[2]);
    else err = grpc_function_handle_default(&handle[i], func_list[1]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  err = grpc_wait_all();
  /* and ignore any errors */

  err = grpc_get_failed_sessionid(&ret_id);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (i = 0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }
  if (ret_id != GRPC_SESSIONID_VOID)
    printf("Success (Report: %s)\n", grpc_error_string(grpc_get_error(ret_id)));
  else
    printf("Failure (The session ID was not returned.)\n");

  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}


/*
 * Call grpc_get_failed_sessionid() repeatedly when
 * there are more than two failed sessions, checking
 * a pointer of that session ID returned one by one,
 * until all of them are popped out.
 */
void error_report_test_7(char *config_file, char **func_list)
{
  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, counter = 0, x = 10;
  printf("Error Reporting Test 7: ");

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[2]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i = 0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j = 0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  err = grpc_wait_all();
  /* and ignore any errors */

  while (1) {
    err = grpc_get_failed_sessionid(&ret_id);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (i = 0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
      grpc_finalize();
      return;
    } else {
      if (ret_id == GRPC_SESSIONID_VOID){
        if (counter == 3) printf("Success\n");
        else printf("Failure (Only %d errors was returned.)\n", counter);
        break;
      } else {
        counter++;
      }
    }
  }

  for (i = 0; i<NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_destruct() failed: %s\n", grpc_error_string(err));
    }
  }
  err = grpc_finalize();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_finalize() failed: %s\n", grpc_error_string(err));
  }
}
