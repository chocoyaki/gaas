INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/Testing/src/GRPC )

generate_diet_tests( DIETGRPCInitializeTests GRPCSeDFixture )
generate_diet_tests( DIETGRPCFinalizeTests GRPCSeDFixture )
generate_diet_tests( DIETGRPCErrorReportTests GRPCSeDFixture )
generate_diet_tests( DIETGRPCMgmtFuncHandleTests GRPCSeDFixture )
generate_diet_tests( DIETGRPCCallTests GRPCSeDFixture )
generate_diet_tests( DIETGRPCCancelTests GRPCSeDFixture )
generate_diet_tests( DIETGRPCProbeTests GRPCSeDFixture )
