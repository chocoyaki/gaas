# try to found SELinux
#
#
# Lamiel Toch, lamiel.toch@ens-lyon.fr




find_library(SELINUX_LIBRARY NAMES selinux)


##############################################################################
# find headers
##############################################################################
find_path(SELINUX_LIBRARY_INCLUDE_DIR selinux/selinux.h)
