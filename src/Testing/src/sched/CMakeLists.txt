add_executable( DIETSchedTestsSeD DIETSchedTestsSeD.cpp )
target_link_libraries( DIETSchedTestsSeD ${DIET_SERVER_LIBRARIES} )
add_library( DIETSchedTestsScheduler SHARED DIETSchedTestsScheduler.cpp )
target_link_libraries( DIETSchedTestsScheduler ${DIET_SERVER_LIBRARIES} )
generate_diet_tests( DIETSchedTestsClient SchedSedFixture SchedClient )
