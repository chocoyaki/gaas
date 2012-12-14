# try to found libdeltacloud
#
#
# Lamiel Toch, lamiel.toch@ens-lyon.fr




find_library(APACHE_DELTACLOUD_LIBRARY NAMES deltacloud)


##############################################################################
# find headers
##############################################################################
find_path(APACHE_DELTACLOUD_INCLUDE_DIR libdeltacloud/libdeltacloud.h)
