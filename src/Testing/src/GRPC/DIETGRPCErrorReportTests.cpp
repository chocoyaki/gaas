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

  for (i=0; i<NCALLS; i++){
    y[i] = 0;
    id[i] = GRPC_SESSIONID_VOID;
  }

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
      fprintf(stderr, "grpc_call_async() failed: %s\n", grpc_error_string(err));
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }
  err = grpc_wait_all();
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_wait_all() failed: %s\n", grpc_error_string(err));
    for (i=0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }
  for (i=0; i<NCALLS; i++) {
    if (y[i] != (x + 1)) {
      fprintf(stderr, "Wrong result: y[%d] = %d\n", i, y[i]);
      for (j=0; j<NCALLS; j++) grpc_function_handle_destruct(&handle[j]);
      grpc_finalize();
      return;
    }
  }

  err = grpc_get_failed_sessionid(&ret_id);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (ID %d got the error: %s)", ret_id, grpc_error_string(err));
    for (i=0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
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

  for (i=0; i<NCALLS; i++) id[i] = GRPC_SESSIONID_VOID;
  
  err = grpc_initialize(config_file);
  if (err != GRPC_NO_ERROR) {
    fprintf(stderr, "grpc_initialize() failed: %s\n", grpc_error_string(err));
    return;
  }
  for (i=0; i<NCALLS; i++) {
    if (i == 0) err = grpc_function_handle_default(&handle[i], func_list[2]);
    else err = grpc_function_handle_default(&handle[i], func_list[1]);
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
  err = grpc_wait_all();
  /* and ignore any errors */

  err = grpc_get_failed_sessionid(&ret_id);
  if (err != GRPC_NO_ERROR) {
    printf("Failure (%s)\n", grpc_error_string(err));
    for (i=0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
    grpc_finalize();
    return;
  }
  if (ret_id != GRPC_SESSIONID_VOID)
    printf("Success (Report: %s)\n", grpc_error_string(grpc_get_error(ret_id)));
  else
    printf("Failure (The session ID was not returned.)\n");

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
  for (i=0; i<NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], func_list[2]);
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
  err = grpc_wait_all();
  /* and ignore any errors */

  while (1) {
    err = grpc_get_failed_sessionid(&ret_id);
    if (err != GRPC_NO_ERROR) {
      printf("Failure (%s)\n", grpc_error_string(err));
      for (i=0; i<NCALLS; i++) grpc_function_handle_destruct(&handle[i]);
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
