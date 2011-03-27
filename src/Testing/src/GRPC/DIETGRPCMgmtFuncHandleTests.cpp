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
