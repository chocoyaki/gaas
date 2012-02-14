#* DIET cmake local file                                                    *#

include_directories(${DIET_SOURCE_DIR}/include)

### A server...
add_executable(server_cori server.c)
target_link_libraries(server_cori ${DIET_SERVER_LIBRARIES})
  
### ... a client...
add_executable(client_cori client.c)
target_link_libraries(client_cori ${DIET_CLIENT_LIBRARIES})

### ... a multithreaded client ... 
add_executable(clientthread_cori client_thread.cc)
target_link_libraries(clientthread_cori ${DIET_CLIENT_LIBRARIES})
  
### ... and a test program
add_executable(coriEasyTester coriEasyTester.cc)
target_link_libraries(coriEasyTester ${DIET_SERVER_LIBRARIES})

### install everything
install(TARGETS server_cori client_cori clientthread_cori coriEasyTester DESTINATION ${EXAMPLE_INSTALL_DIR}/cori)
