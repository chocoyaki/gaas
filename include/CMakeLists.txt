#
# DIET cmake local file
#

INSTALL_FILES(
  /include/
  FILES
    DIET_client.h
    DIET_data.h
    DIET_mutex.h
    DIET_grpc.h
    DIET_server.h
)

# FIXME: DIET_config.h is not installed since it would conflict with cmake's
#        generated one...
