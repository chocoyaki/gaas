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

  for (i=0; i<NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    if (i == 0) x[i] = 0;
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
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
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
      for (i=0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
      grpc_finalize();
      return;
    }
  }

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

  for (i=0; i<NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    x[i] = 10;
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
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_probe_or(id, NCALLS, &ret_id);
  if (err == GRPC_NONE_COMPLETED) {
    if(ret_id == GRPC_SESSIONID_VOID) printf("Success\n");
    else printf("Failure (The returned ID was not GRPC_SESSIONID_VOID.)\n");
  } else if (err == GRPC_NO_ERROR) {
    printf("Failure (All sessions were completed before calling grpc_probe_or().)");
  } else {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (i=0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }

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

  for (i=0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;

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
