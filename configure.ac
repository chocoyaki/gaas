#****************************************************************************#
#* DIET autoconf configure                                                  *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Christophe PERA (Christophe.Pera@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.13  2003/06/23 13:13:27  pcombes
#* Re-Submit BLAS and ScaLAPACK detection to "enable" options.
#* Change configuration summary (more complete)
#*
#* Revision 1.12  2003/06/16 17:51:06  pcombes
#* Remove src/examples/dmat_manips_async.
#*
#* Revision 1.11  2003/06/02 09:14:53  cpera
#* Manage asynchronize samples.
#*
#* Revision 1.9  2003/05/16 17:29:28  pcombes
#* Clean macros and messages. Set omniORB_min_version (not used yet)
#*
#* Revision 1.6  2003/04/10 14:59:23  pcombes
#* Add modules doc (disabled by default), examples (forced),
#* examples/BLAS and examples/ScaLAPCAK (both disabled by default).
#*
#* Revision 1.3  2003/02/17 15:34:56  cpera
#* Use omniorb.m4 and detect new platforms.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#****************************************************************************#

define(DIET_version,        0.7_beta)
define(omniORB_min_version, 3)
define(FAST_min_version,    0.4.3)


dnl ##########################################################################
dnl #### Process this file with autoconf to produce a configure script.
dnl ##########################################################################
AC_PREREQ([2.55])
ACI_PREREQ(2003.01.29)
AC_INIT(DIET, DIET_version, diet-dev@ens-lyon.fr)
AC_CONFIG_SRCDIR([include/DIET_data.h])
AC_REVISION($Revision$)
AC_CONFIG_HEADERS([include/DIET_config.h])
AC_PREFIX_DEFAULT(`pwd`/install)


dnl ##########################################################################
dnl #### Create templates for config.h.in header.
dnl ##########################################################################
AH_TEMPLATE([HAVE_OMNIORB], [Define use of omniORB.])
AH_TEMPLATE([HAVE_CICHLID], [Define use of Cichlid.])
AH_TEMPLATE([HAVE_STATISTICS], [Define use of STATISTICS library.])
AH_TEMPLATE([HAVE_BLAS], [Define use of BLAS library for DIET examples.])
dnl AH_TEMPLATE([HAVE_PBLAS], [Define use of PBLAS library for DIET examples.])
dnl AH_TEMPLATE([HAVE_MPI], [Define use of MPI library for DIET examples.])
AH_TEMPLATE([HAVE_SCALAPACK], [Define use of SCALAPACK library \
                              for DIET examples.])
dnl AH_TEMPLATE([HAVE_GSL], [Define use of GSL library for DIET examples.])
AH_TEMPLATE([HAVE_MULTI_MA], [using multi MA architecture.])
AH_TOP([#ifndef _DIET_CONFIG_H_
#define _DIET_CONFIG_H_])
AH_BOTTOM([#endif // _DIET_CONFIG_H_])


AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE[]
AM_MAINTAINER_MODE

dnl ##########################################################################
dnl #### check OS target
dnl ** Platform name | according to omniORB configure.ac
dnl ##########################################################################
plat_name="Platform_Unknown"
plat_def="__unknown_platform__"
os_version="0"
case "$host" in
  *-*-linux-*)   plat_name="Linux";    plat_def="__linux__";    os_v="2";;
  *-*-solaris*)  plat_name="SunOS";    plat_def="__sunos__";    os_v="5";;
  *-*-osf3*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="3";;
  *-*-osf4*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="4";;
  *-*-osf5*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="5";;
  *-*-hpux10*)   plat_name="HPUX";     plat_def="__hpux__";     os_v="10";;
  *-*-hpux11*)   plat_name="HPUX";     plat_def="__hpux__";     os_v="11";;
  *-*-nextstep*) plat_name="NextStep"; plat_def="__nextstep__"; os_v="3";;
  *-*-openstep*) plat_name="NextStep"; plat_def="__nextstep__"; os_v="3";;
  *-*-irix*)     plat_name="IRIX";     plat_def="__irix__";     os_v="6";;
  *-*-aix*)      plat_name="AIX";      plat_def="__aix__";      os_v="4";;
  *-*-darwin*)   plat_name="Darwin";   plat_def="__darwin__";   os_v="1";;
  *-*-freebsd3*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="3";;
  *-*-freebsd4*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="4";;
  *-*-sco*)      plat_name="OSR5";     plat_def="__osr5__";     os_v="5";;
esac
AC_SUBST(HOST_NAME, $ac_hostname)
AC_SUBST(PLATFORM_NAME, $plat_name)
AC_SUBST(PLATFORM_DEFINE, $plat_def)
AC_SUBST(OSVERSION, $os_v)

dnl ##########################################################################
dnl #### check OS target
dnl ** Processor | according to omniORB configure.ac
dnl ##########################################################################
proc_name="UnknownProcessor"
proc_def="__processor_unknown__"
case "$host" in
  i?86-*)   proc_name="x86Processor";     proc_def="__x86__";;
  sparc-*)  proc_name="SparcProcessor";   proc_def="__sparc__";;
  alpha*)   proc_name="AlphaProcessor";   proc_def="__alpha__";;
  m68k-*)   proc_name="m68kProcessor";    proc_def="__m68k__";;
  mips*)    proc_name="IndigoProcessor";  proc_def="__mips__";;
  arm-*)    proc_name="ArmProcessor";     proc_def="__arm__";;
  s390-*)   proc_name="s390Processor";    proc_def="__s390__";;
  ia64-*)   proc_name="ia64Processor";    proc_def="__ia64__";;
  hppa*)    proc_name="HppaProcessor";    proc_def="__hppa__";;
  powerpc*) proc_name="PowerPCProcessor"; proc_def="__powerpc__";;
esac
AC_SUBST(PROCESSOR_NAME, $proc_name)
AC_SUBST(PROCESSOR_DEFINE, $proc_def)


dnl ###########################################################################
# DIET modules listing. 
dnl ###########################################################################
ACI_MODULE([DIET],,[omniORB FAST],[src doc],force)
  ACI_MODULE([doc],[documentation about DIET],,,no)
  ACI_MODULE([src],,,[agent client SeD examples],force)
    ACI_MODULE([agent],[DIET_MA and DIET_LA],,,force)
    ACI_MODULE([client],[the client library],,,force)
    ACI_MODULE([SeD],[the DIET Server Daemon],,,force)
    ACI_MODULE([examples],[basic DIET examples],[SeD client],[BLAS ScaLAPACK])
      ACI_MODULE([BLAS],[an example for calling BLAS functions through DIET],
	         [examples BLAS],,no)
      ACI_MODULE([ScaLAPACK],
	         [an example for calling ScaLAPACK functions through DIET],
	         [examples ScaLAPACK],,no)

dnl ###########################################################################
# Various compilation features
dnl ###########################################################################
AC_ARG_ENABLE(Cichlid,
	      AC_HELP_STRING([--enable-Cichlid],
	                 [enable generation of communication logs for Cichlid]))
if test "x$enable_CICHLID" = "xyes"; then
  AC_DEFINE(HAVE_CICHLID,1,[Define use of communication tools])
fi

AC_ARG_ENABLE(stats,
	      AC_HELP_STRING([--enable-stats],
			     [enable generation of statistics logs]))
if test "x$enable_stats" = "xyes"; then
  AC_DEFINE(HAVE_STATISTICS)
  IDLFLAGS="$IDLFAGS -DHAVE_STATISTICS"
  HAVE_STATISTICS=yes
fi

AC_ARG_ENABLE(multi-MA,
	      AC_HELP_STRING([--enable-multi-MA],
			     [enable multi-MA architecture]))
if test "x$enable_MULTI_MA" = "xyes"; then
  AC_DEFINE(HAVE_MULTI_MA)
fi


dnl ###########################################################################
dnl Load anything under acmacros/*.m4
dnl ###########################################################################
ACLOCAL="$ACLOCAL -I acmacros"


dnl ###########################################################################
# Checks for programs.
dnl ###########################################################################
AC_PROG_CXX
AC_LANG_CPLUSPLUS
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_attr_getdetachstate])

dnl ###########################################################################
# Search the ORB to use
dnl ###########################################################################
AC_OMNIORB_DIET(omniORB_min_version,,,,
		$proc_name,$proc_def,$plat_name,$plat_def,$os_v,
		HAVE_OMNIORB=yes,HAVE_OMNIORB=no)
if test "x$HAVE_OMNIORB" != "xyes"; then
  AC_MSG_ERROR([could not detect omniORB omniORB_min_version or higher])
fi

# AC_TAO ???

dnl ###########################################################################
# Search for the FAST Library
dnl ###########################################################################
AM_PATH_FAST(FAST_min_version,
	     [HAVE_FAST=true
	      AC_DEFINE(HAVE_FAST,1,[Define use of the FAST library])],
	     [HAVE_FAST=false
	      AC_MSG_WARN(Use of FAST library is disabled - either by you or)
              AC_MSG_WARN(because it was not found: all performance evaluations)
	      AC_MSG_WARN(will be set to infinity.)])

dnl ###########################################################################
dnl ####[ search for the BLAS Library ]
dnl ###########################################################################
if test "x$enable_BLAS" = "xyes"; then
  ACI_PACKAGE([BLAS],[BLAS],[dgemm_],
              [-lcblas -lblas -lblas2 -latlas!-lblas!-llapack!-lm!-lblas!\
               -lcblas!-llapack!-lm!-L/usr/lib/atlas ],,,:)
fi

dnl ###########################################################################
dnl ####[ search for the ScaLAPACK Library ]
dnl ###########################################################################
if test "x$enable_ScaLAPACK" = "xyes"; then
  ACI_PACKAGE([ScaLAPACK],[ScaLAPACK],[pdgemm_],[-latlas],,,:)
fi

dnl ###########################################################################
dnl ####[ search for the BLACS Library ]
dnl ###########################################################################
#ACI_PACKAGE([BLACS],[the BLACS library: used by DIET samples and so \
#		necessary if they are compiled], 
#		[ldap_bind],
#		[-lldap -lldap!-L$with_OPENLDAP/lib!-I$with_OPENLDAP/include],
#		[ldap.h])

dnl ###########################################################################
dnl ####[ check for some programs ]
dnl ###########################################################################
AC_CHECK_PROG(BASH, bash, `which bash`, /bin/sh)
WARNING="This file is generated, do not edit"
AC_SUBST(WARNING)

dnl ###########################################################################
dnl ####[ select the right modules ]
dnl ###########################################################################
ACI_MODULES_VERIFY([DIET]) 
AC_SUBST(MODULES_DIET)
AC_SUBST(MODULES_agent)
AC_SUBST(MODULES_SeD)
AC_SUBST(MODULES_client)
AC_SUBST(MODULES_examples)
AC_SUBST(MODULES_doc)

AC_SUBST(DIST_MODULES_DIET)
AC_SUBST(DIST_MODULES_agent)
AC_SUBST(DIST_MODULES_SeD)
AC_SUBST(DIST_MODULES_client)
AC_SUBST(DIST_MODULES_examples)
AC_SUBST(DIST_MODULES_doc)

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)

AC_SUBST(IDLFLAGS)

AC_SUBST(prefix)
AC_SUBST(topsrcdir)
AC_LANG(C++)


dnl ###########################################################################
# Checks for header files.
dnl ###########################################################################
dnl AC_HEADER_STAT   # not very useful, yet, although sys/stat.h is included
dnl AC_HEADER_STDC   # we should learn using the defined variables ...
dnl AC_HEADER_TIME   # we should learn using the defined variables ...
AC_CHECK_HEADERS([alloca.h assert.h iostream fstream limits.h math.h])


dnl ###########################################################################
# Checks for typedefs, structures, and compiler characteristics.
dnl ###########################################################################
AC_C_CONST
AC_C_INLINE
dnl AC_TYPE_PID_T # unused

# FIXME: we should get the sizes of all types, to map DIET base types
#        (DIET_INT, etc.)
AC_TYPE_SIZE_T

dnl ###########################################################################
# Checks for library functions.
dnl ###########################################################################
AC_FUNC_MALLOC
AC_FUNC_REALLOC
dnl AC_TYPE_SIGNAL # unused yet
AC_FUNC_STAT
AC_FUNC_STRTOD # used in BLAS and ScaLAPACK clients
AC_CHECK_FUNCS([gethostname gettimeofday strdup strrchr strtok_r])


dnl ###########################################################################
dnl Create target output (Makefile(s) ...) 
dnl ###########################################################################
#include/DIET_config.h
AC_CONFIG_FILES([include/Makefile.inc
		config.mk
		Makefile
		src/Makefile
                src/CORBA/Makefile
                src/CORBA/idl/Makefile
                src/utils/Makefile
                src/SeD/Makefile
                src/agent/Makefile
                src/client/Makefile
		src/examples/Makefile
                src/examples/BLAS/Makefile
                src/examples/dmat_manips/Makefile
                src/examples/file_transfer/Makefile
                src/examples/ScaLAPACK/Makefile
		doc/Makefile
		doc/CS/Makefile
		doc/UM/Makefile])
AC_OUTPUT

dnl ###########################################################################
dnl #### Print general info on DIET 
dnl ###########################################################################
echo "" | tee -a config.log
echo "DIET successfully configured as follows:" | tee -a config.log

# CICHLID
if test "x$enable_Cichlid" = "xyes"
then
  echo " - Cichlid log:     yes" | tee -a config.log
fi

# DOCUMENTS
if test "x$enable_doc" = "xyes"
then
  echo " - documents:       yes" | tee -a config.log
fi

# EXAMPLES
printf " - examples:        " | tee -a config.log
if test "x$enable_examples" = "xno"
then
  echo none | tee -a config.log
else
  printf "dmat_manips, file_transfer" | tee -a config.log
  if test "x$HAVE_BLAS" = "xyes"
  then
    printf ", BLAS" | tee -a config.log
  fi
  if test "x$HAVE_ScaLAPACK" = "xyes"
  then
    printf ", ScaLAPACK" | tee -a config.log
  fi
  echo | tee -a config.log
fi

# FAST
printf " - FAST:            " | tee -a config.log
if test "x$HAVE_FAST" = "xyes"
then
  echo "found $FAST_CONFIG" | tee -a config.log
else
  echo "not found" | tee -a config.log
fi

# ORB
printf " - ORB:             " | tee -a config.log
if test "x$HAVE_OMNIORB" = "xyes"
then
  echo "omniORB $OMNIORB_VERSION in $OMNIORB_HOME" | tee -a config.log
else if test "x$HAVE_TAO" = "xyes"
then
  echo "TAO $TAO_VERSION in $TAO_HOME" | tee -a config.log
fi fi

# PREFIX
echo " - prefix:          $prefix" | tee -a config.log

# MAINTAINER MODE
if test "x$enable_maintainer_mode" = "xyes"
then
  echo " - maintainer mode: yes" | tee -a config.log
fi

# MULTI MA
printf " - multi-MA:        " | tee -a config.log
if test "x$HAVE_MULTI_MA" = "xyes"
then
  echo "yes" | tee -a config.log
else
  echo "no" | tee -a config.log
fi

# STATISTICS
if test "x$HAVE_STATISTICS" = "xyes"
then
  echo " - statistics log:  yes" | tee -a config.log
fi

# PLATFORM
echo " - target platform: $target" | tee -a config.log

echo | tee -a config.log

echo "Please run make help to get compilation instructions."

exit 0;
