#include <cstring>
#include <boost/scoped_ptr.hpp>

#include <DIET_client.h>
#include <DIET_grpc.h>

#include "fixtures.hpp"
#include "utils.hpp"
#include "configGRPC.hpp"

BOOST_FIXTURE_TEST_SUITE(GRPCProbeTests,
                         GRPCSeDFixture)



/*
 * Call grpc_probe() with a session ID which is completed,
 * checking GRPC_NO_ERROR returned.
 */
BOOST_AUTO_TEST_CASE(probe_test_1) {
  BOOST_TEST_MESSAGE("-- Test: Probe Test 1");

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 3, y = 0;
  utils::ClientArgs c("probe_test_1", "client_testing.cfg");

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_function_handle_default(&handle, strdup(func_list[0]));
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_call_async(&handle, &id, x, &y);
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  while (1) {
    err = grpc_probe(id);
    if (err != GRPC_NO_ERROR) {
      BOOST_CHECK_EQUAL(err, GRPC_NOT_COMPLETED);
      break;
    }
  }

  err = grpc_wait(id);
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_probe() with a session ID which is not completed,
 * checking GRPC_NOT_COMPLETED returned.
 */
BOOST_AUTO_TEST_CASE(probe_test_2) {
  BOOST_TEST_MESSAGE("-- Test: Probe Test 2");

  grpc_function_handle_t handle;
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  int x = 10;
  utils::ClientArgs c("probe_test_2", "client_testing.cfg");

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_function_handle_default(&handle, strdup(func_list[1]));
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_call_async(&handle, &id, x);
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_probe(id);
  BOOST_CHECK((err == GRPC_NOT_COMPLETED) || (err == GRPC_NO_ERROR));

  err = grpc_wait(id);
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_function_handle_destruct(&handle);
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_probe() with an invalid session ID,
 * checking GRPC_INVALID_SESSION_ID returned.
 */
BOOST_AUTO_TEST_CASE(probe_test_3) {
  BOOST_TEST_MESSAGE("-- Test: Probe Test 3");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;
  utils::ClientArgs c("probe_test_3", "client_testing.cfg");

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_probe(id);
  BOOST_CHECK_EQUAL(err, GRPC_INVALID_SESSION_ID);

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_probe() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returnd.
 */
BOOST_AUTO_TEST_CASE(probe_test_4) {
  BOOST_TEST_MESSAGE("-- Test: Probe Test 4");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID;

  err = grpc_probe(id);
  BOOST_CHECK_EQUAL(err, GRPC_NOT_INITIALIZED);
}


/*
 * Call grpc_probe_or() with a session ID array which contains
 * at least one completed session, checking GRPC_NO_ERROR
 * returned with a pointer of the probed session ID.
 */
BOOST_AUTO_TEST_CASE(probe_test_5) {
  BOOST_TEST_MESSAGE("-- Test: Probe Test 5");

  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, x[NCALLS];

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    if (i == 0) {
      x[i] = 0;
    } else {
      x[i] = 10;
    }
  }

  utils::ClientArgs c("probe_test_5", "client_testing.cfg");

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

  err = grpc_probe_or(id, NCALLS, &ret_id);
  while (1) {
    BOOST_CHECK(err != GRPC_NONE_COMPLETED);
    if (err == GRPC_NO_ERROR) {
      break;
    }
  }

  err = grpc_wait_all();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_probe_or() with a session ID array which
 * does not contain any of completed sessions, checking
 * GRPC_NONE_COMPLETED returned with an invalid session ID.
 */
BOOST_AUTO_TEST_CASE(probe_test_6) {
  BOOST_TEST_MESSAGE("-- Test: Probe Test 6");

  grpc_function_handle_t handle[NCALLS];
  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id = GRPC_SESSIONID_VOID;
  int i, j, x[NCALLS];

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
    x[i] = 10;
  }

  utils::ClientArgs c("probe_test_6", "client_testing.cfg");

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

  err = grpc_probe_or(id, NCALLS, &ret_id);
  BOOST_CHECK_EQUAL(err, GRPC_NONE_COMPLETED);
  BOOST_CHECK_EQUAL(ret_id, GRPC_SESSIONID_VOID);

  err = grpc_wait_all();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  for (i = 0; i < NCALLS; i++) {
    err = grpc_function_handle_destruct(&handle[i]);
    BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
  }

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_probe_or() with a session ID array which contains
 * an invalid session ID, checking GRPC_INVALID_SESSION_ID
 * returned.
 */
BOOST_AUTO_TEST_CASE(probe_test_7) {
  BOOST_TEST_MESSAGE("-- Test: Probe Test 7");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id[NCALLS], ret_id;
  int i;

  for (i = 0; i < NCALLS; i++) {
    id[i] = GRPC_SESSIONID_VOID;
  }

  utils::ClientArgs c("probe_test_7", "client_testing.cfg");

  err = grpc_initialize(c.config());
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);

  err = grpc_probe_or(id, NCALLS, &ret_id);
  BOOST_CHECK_EQUAL(err, GRPC_INVALID_SESSION_ID);

  err = grpc_finalize();
  BOOST_CHECK_EQUAL(err, GRPC_NO_ERROR);
}


/*
 * Call grpc_probe_or() before calling grpc_initialize(),
 * checking GRPC_NOT_INITIALIZED returned.
 */
BOOST_AUTO_TEST_CASE(probe_test_8) {
  BOOST_TEST_MESSAGE("-- Test: Probe Test 8");

  grpc_error_t err = GRPC_NO_ERROR;
  grpc_sessionid_t id = GRPC_SESSIONID_VOID, ret_id = GRPC_SESSIONID_VOID;

  err = grpc_probe_or(&id, 1, &ret_id);
  BOOST_CHECK_EQUAL(err, GRPC_NOT_INITIALIZED);
}


BOOST_AUTO_TEST_SUITE_END()
