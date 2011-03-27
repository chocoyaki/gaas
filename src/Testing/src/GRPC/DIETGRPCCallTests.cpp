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
