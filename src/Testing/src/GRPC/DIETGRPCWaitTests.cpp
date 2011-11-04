#include <cstring>
#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"
#include "configGRPC.hpp"

BOOST_FIXTURE_TEST_SUITE(GRPCWaitTests,
                         GRPCSeDFixture)





/*
 * Call grpc_wait() with a valid session ID to be synchronized,
 * checking GRPC_NO_ERROR returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_1) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 1");

  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i, j, x = 3, y = 0;
  utils::ClientArgs c("wait_test_1", "client_testing.cfg");

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
  }

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], strdup(func_list[0]));
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_wait(id[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
    BOOST_CHECK_EQUAL(y, x + 1);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_wait() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_2) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 2");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  utils::ClientArgs c("wait_test_2", "client_testing.cfg");

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_wait(id);
  BOOST_CHECK_EQUAL(err, GRPC_INVALID_SESSION_ID);

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_wait() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_3) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 3");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;

  err = grpc_wait(id);
  BOOST_CHECK_EQUAL(err, GRPC_NOT_INITIALIZED);
}


/*
 * Call grpc_wait_and() with an array of the valid session IDs,
 * checking GRPC_NO_ERROR returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_4) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 4");

  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i, j, x = 3, y = 0;
  utils::ClientArgs c("wait_test_4", "client_testing.cfg");

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
  }

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], strdup(func_list[0]));
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_wait_and(id, NCALLS);
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  BOOST_CHECK_EQUAL(y, x + 1);

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_wait_and() with a session ID array which contains
 * at least one invalid ID, checking GRPC_INVALID_SESSION_ID
 * returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_5) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 5");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i;
  utils::ClientArgs c("wait_test_5", "client_testing.cfg");

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
  }

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_wait_and(id, NCALLS);
  BOOST_CHECK_EQUAL(err, GRPC_INVALID_SESSION_ID);

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_wait_and() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_6) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 6");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;

  err = grpc_wait_and(&id, 1);
  BOOST_CHECK_EQUAL(err, GRPC_NOT_INITIALIZED);
}


/*
 * Call grpc_wait_or() with a valid session ID array,
 * checking GRPC_NO_ERROR returned with a pointer of
 * the completed session ID.
 */
BOOST_AUTO_TEST_CASE(wait_test_7) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 7");

  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, found, x[NCALLS];
  utils::ClientArgs c("wait_test_7", "client_testing.cfg");

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    if (i == 0) {
      x[i] = 1;
    } else {
      x[i] = 10;
    }
  }

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], strdup(func_list[1]));
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  for (i = NCALLS; i > 0; i--) {
    err = grpc_wait_or(id, i, &ret_id);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

    /* Find the returned id and replace it with the last id */
    found = 0;
    for (j = 0; j < i; j++) {
      if (ret_id == id[j]) {
        id[j] = id[i - 1];
        id[i - 1] = ret_id;
        found = 1;
        break;
      }
    }
    BOOST_CHECK(found != 0);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_wait_or() with a session ID array which
 * contains at least one invalid ID, checking
 * GRPC_INVALID_SESSION_ID returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_8) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 8");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i;
  utils::ClientArgs c("wait_test_8", "client_testing.cfg");

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
  }

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_wait_or(id, NCALLS, &ret_id);
  BOOST_CHECK_EQUAL(err, GRPC_INVALID_SESSION_ID);

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_wait_or() before calling grpc_wait_or(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_9) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 9");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID, ret_id = GRPC_SESSIONID_VOID;

  err = grpc_wait_or(&id, 1, &ret_id);
  BOOST_CHECK_EQUAL(err, GRPC_NOT_INITIALIZED);
}


/*
 * Call grpc_wait_all() in right way, checking
 * GRPC_NO_ERROR returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_10) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 10");

  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS];
  int i, j, x = 3, y[NCALLS];
  utils::ClientArgs c("wait_test_10", "client_testing.cfg");

  for (i = 0; i < NCALLS; i++) {
    y[i] = 0;
  }

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], strdup(func_list[0]));
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_wait_all();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  for (i = 0; i < NCALLS; i++) {
    BOOST_CHECK_EQUAL(y[i], x + 1);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_wait_all() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_11) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 11");

  grpc_error_t err = GRPC_NO_ERROR;

  err = grpc_wait_all();
  BOOST_CHECK_EQUAL(err, GRPC_NOT_INITIALIZED);
}


/*
 * Call grpc_wait_any() in right way, checking GRPC_NO_ERROR
 * returned with an ID pointer of the session which is
 * completed.
 */
BOOST_AUTO_TEST_CASE(wait_test_12) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 12");

  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, counter = 0, x = 3, y[NCALLS];
  utils::ClientArgs c("wait_test_12", "client_testing.cfg");

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
  }

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_default(&handle[i], strdup(func_list[0]));
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_call_async(&handle[i], &id[i], x, &y[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  while (1) {
    err = grpc_wait_any(&ret_id);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
    counter++;
    if (counter == NCALLS) {
      break;
    }
  }

  for (i = 0; i < NCALLS; i++) {
    BOOST_CHECK_EQUAL(y[i], x + 1);
  }

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_wait_any() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE(wait_test_13) {
  BOOST_TEST_MESSAGE("-- Test: Wait Test 13");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  printf("Wait Test 13: ");

  err = grpc_wait_any(&id);
  BOOST_CHECK_EQUAL(err, GRPC_NOT_INITIALIZED);
}


BOOST_AUTO_TEST_SUITE_END()
