// TODO


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

  for (i=0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  for (i=0; i<NCALLS; i++) {
    err = grpc_wait(id[i]);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
    if (y != (x + 1)) {
      printf("Failure (wrong result: y = %d)\n", y);
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  } 
  printf("Success\n");

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

  for (i=0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_wait_and(id, NCALLS);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
    grpc_finalize();
    return;
  }
  if (y != (x + 1)) {
    printf("Failure (wrong result: y = %d)\n", y);
    for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
    grpc_finalize();
    return;
  }
  printf("Success\n");

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

  for (i=0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

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
  
  for (i=0; i<NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    if (i == 0) x[i] = 1;
    else x[i] = 10;
  }

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[1]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x[i]);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  for (i=NCALLS; i>0; i--) {
    err = grpc_wait_or(id, i, &ret_id);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }

    /* Find the returned id and replace it with the last id */
    found = 0;
    for (j=0; j<i; j++) {
      if (ret_id == id[j]) {
        id[j] = id[i-1];
	id[i-1] = ret_id;
	found = 1;
	break;
      }
    }
    if (found == 0) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  printf("Success\n");

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

  for (i=0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

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

  for (i=0; i<NCALLS; i++) y[i] = 0;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
    grpc_finalize();
    return;
    }
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y[i]);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_wait_all();
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
    grpc_finalize();
    return;
  }
  for (i=0; i<NCALLS; i++) {
    if (y[i] != (x + 1)) {
      printf("Failure (wrong result: y[%d] = %d)\n", i, y[i]);
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  printf("Success\n");

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

  for (i=0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[0]);
    if (err != GRPC_NO_ERROR) {
      fprintf(stderr, "grpc_function_handle_default() failed: %s\n", grpc_error_string(err));
      grpc_finalize();
      return;
    }
  }
  for (i=0; i<NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y[i]);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  while (1) {
    err = grpc_wait_any(&ret_id);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
    counter++;
    if (counter == NCALLS) break;
  }
  for (i=0; i<NCALLS; i++) {  
    if (y[i] != (x + 1)) {
      printf("Failure (wrong result: y[%d] = %d)\n", i, y[i]);
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  printf("Success\n");

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
