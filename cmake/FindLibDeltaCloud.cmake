# try to found libdeltacloud
#
#
# Lamiel Toch, lamiel.toch@ens-lyon.fr




find_library(DELTACLOUD_LIBRARY NAMES deltacloud)


##############################################################################
# find headers
##############################################################################
find_path(DELTACLOUD_INCLUDE_DIR libdeltacloud/libdeltacloud.h)
