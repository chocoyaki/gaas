INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/Testing/src/GRPC )

generate_diet_tests( DIETGRPCInitializeTests GRPCSeDFixture GRPCInitializeTests )
generate_diet_tests( DIETGRPCFinalizeTests GRPCSeDFixture GRPCFinalizeTests )
generate_diet_tests( DIETGRPCErrorReportTests GRPCSeDFixture GRPCErrorReportTests )
generate_diet_tests( DIETGRPCMgmtFuncHandleTests GRPCSeDFixture GRPCMgmtFuncHandleTests )
generate_diet_tests( DIETGRPCCallTests GRPCSeDFixture GRPCCallTests )
generate_diet_tests( DIETGRPCCancelTests GRPCSeDFixture GRPCCancelTests )
generate_diet_tests( DIETGRPCProbeTests GRPCSeDFixture GRPCProbeTests )
#generate_diet_tests( DIETGRPCWaitTests GRPCSeDFixture GRPCProbeTests )


# TODO: uncomment tests with calls to async_call as they currently hang indefinitively