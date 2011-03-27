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

  for (i=0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[3]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_cancel_all();
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (i=0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }
  printf("Success\n");

  err = grpc_wait_all();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait_all() failed: %s\n", grpc_error_string(err));
  }
  for (i=0; i<NCALLS; i++) {
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
