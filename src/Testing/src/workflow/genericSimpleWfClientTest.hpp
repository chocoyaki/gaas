#ifndef __GENERICSIMPLEWFCLIENTTEST_HPP__
#define __GENERICSIMPLEWFCLIENTTEST_HPP__

void
genericSimpleWorkflowClient(const std::string &wfFileName,
                            const std::string &wfName,
                            const wf_level_t &wfType,
                            const std::string &dataFileName = "",
                            const std::string &dataOutFileName = "data_out.xml",
                            const std::string &transcriptFileName = "") {
  BOOST_TEST_MESSAGE("Generic Workflow client.\nParameters:");
  BOOST_TEST_MESSAGE("Workflow filename: " << wfFileName);
  BOOST_TEST_MESSAGE("Workflow name: " << wfName);
  if (wfType == DIET_WF_FUNCTIONAL) {
    BOOST_TEST_MESSAGE("Workflow type: DIET_WF_FUNCTIONAL");
  } else {
    BOOST_TEST_MESSAGE("Workflow type: DIET_WF_DAG");
  }
  BOOST_TEST_MESSAGE("Data filename: " << dataFileName);
  BOOST_TEST_MESSAGE("Data out filename: " << dataOutFileName);
  BOOST_TEST_MESSAGE("Transcript filename: " << transcriptFileName);

  utils::ClientArgs c("files", "client_testing_wf.cfg");

  diet_error_t error = diet_initialize(c.config(), c.argc(), c.argv());

  // check if diet_initialize don't return any error
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_initialize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));


  /*
   * Allocate the workflow profile
   */
  diet_wf_desc_t *profile;
  profile = diet_wf_profile_alloc(wfFileName.c_str(), wfName.c_str(), wfType);
  BOOST_CHECK(profile != NULL);

  if (wfType == DIET_WF_FUNCTIONAL) {
    /*
     * For functional workflows ONLY
     * Defines which file is used to provide the data to instanciate the wf
     */
    diet_wf_set_data_file(profile, dataFileName.c_str());

    /*
     * For workflow restart
     * Defines which file is used to store the execution transcriptFileName
     * (file will be overwritten if existing)
     */
    if (transcriptFileName != "") {
      diet_wf_set_transcript_file(profile, transcriptFileName.c_str());
    }
  }


  /* Execute workflow */
  error = diet_wf_call(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);

  /* Save output data */
  if (wfType == DIET_WF_FUNCTIONAL) {
    error = diet_wf_save_data_file(profile, dataOutFileName.c_str());
    BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  }

  /* Display result */
  error = diet_wf_print_results(profile);
  BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);


  if (transcriptFileName == "") {
    diet_wf_free(profile);
  } else {
    error = diet_wf_save_transcript_file(profile, transcriptFileName.c_str());
    BOOST_CHECK_EQUAL(GRPC_NO_ERROR, error);
  }

  // now we're good
  BOOST_TEST_MESSAGE("-- Now calling diet_finalize ");
  diet_finalize();
  BOOST_REQUIRE_MESSAGE(GRPC_NO_ERROR == error,
                        "diet_finalize() should return "
                        << diet_error_string(GRPC_NO_ERROR)
                        << " instead of "
                        << diet_error_string(error));
} // genericSimpleWorkflowClient


#endif // ifndef __GENERICSIMPLEWFCLIENTTEST_HPP__
