#****************************************************************************#
#* $Id$                *#
#* DIET autoconf configure                                                  *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Christophe   PERA          - LIP ENS-Lyon (France)                  *#
#*                                                                          *#
#*  This is part of DIET software.                                          *#
#*  Copyright (C) 2002 ReMaP/LIFC/INRIA                                     *#
#*                                                                          *#
#****************************************************************************#
#*
#* $Log$
#* Revision 1.3  2003/02/17 15:34:56  cpera
#* Use omniorb.m4 and detect new platforms.
#*
#* Revision 1.2  2003/02/14 16:20:33  cpera
#* DIET instead of diet.
#*
#* Revision 1.1  2003/02/14 10:31:29  cpera
#* add files used by autoconf/automake/libtool.
#*
#*
#****************************************************************************#

dnl ###########################################################################
dnl #### Process this file with autoconf to produce a configure script.
dnl ###########################################################################
AC_PREREQ([2.55])
ACI_PREREQ(2003.01.29)
AC_INIT(DIET, 0.7, diet-dev@ens-lyon.fr)
AC_CONFIG_SRCDIR([src/agent/agent_impl.hh])
AC_REVISION($Revision$)
AC_CONFIG_HEADERS([include/DIET_config.h])


dnl ###########################################################################
dnl #### Create templates for config.h.in header.
dnl ###########################################################################
AH_TEMPLATE([HAVE_omniORB], [Define use of OMNIORB.])
AH_TEMPLATE([HAVE_CICHLID], [Define use of CICHLID .])
AH_TEMPLATE([HAVE_STATISTICS], [Define use of STATISTIC library.])
AH_TEMPLATE([HAVE_BLAS], [Define use of BLAS library for DIET examples.])
AH_TEMPLATE([HAVE_PBLAS], [Define use of PBLAS library for DIET examples.])
AH_TEMPLATE([HAVE_MPI], [Define use of MPI library for DIET examples.])
AH_TEMPLATE([HAVE_SCALAPACK], [Define use of SCALAPACK library \
                              for DIET examples.])
AH_TEMPLATE([HAVE_GSL], [Define use of GSL library for DIET examples.])
AH_TEMPLATE([HAVE_MULTI_MA], [using multi MA architecture.])
AH_TOP([#ifndef _DIET_CONFIG_H_
#define _DIET_CONFIG_H_])
AH_BOTTOM([#endif // _DIET_CONFIG_H_])



AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE[]
AM_MAINTAINER_MODE


AC_ARG_ENABLE(multi-MA,[  --enable-multi-MA       enable multi MA architecture],
              desireMA=true,desireMA=false)
if test "x$desireMA" = "xtrue"
	then
	echo HAVE_MULTI_MA
	AC_DEFINE(HAVE_MULTI_MA)
fi


dnl ###########################################################################
dnl #### check OS target
dnl ** Platform name | according to omniORB configure.ac
dnl ###########################################################################
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
AC_SUBST(PLATFORM_NAME, $plat_name)
AC_SUBST(PLATFORM_DEFINE, $plat_def)
AC_SUBST(OSVERSION, $os_v)
dnl ###########################################################################
dnl #### check OS target
dnl ** Processor | according to omniORB configure.ac
dnl ###########################################################################
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
# Initialize the test suite and build position independent wrappers.
dnl ###########################################################################
#AC_CONFIG_TESTDIR([tests])
#AC_CONFIG_FILES([tests_auto/Makefile tests_auto/atlocal])

dnl ###########################################################################
dnl DIET modules listing. 
dnl ###########################################################################
ACI_MODULE([DIET],[The middleware you are trying to use ;)],[omniORB] [FAST],[agent client SeD],,[CORBA  utils])
  ACI_MODULE([agent],[Master and Local agent;],)
  ACI_MODULE([client],[client for master agent;],)
  ACI_MODULE([SeD],[basic DIET service server;],)
  ACI_MODULE([examples],[basic DIET service server;],)

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
dnl ####[ search omniORB lib and bin ]
dnl ###########################################################################
AC_OMNIORB_DIET(,,,,[testing omniorb OK],[testing omniORB NOK])

dnl ###########################################################################
dnl ####[ search the FAST Library ]
dnl ###########################################################################
AM_PATH_FAST(,[AC_DEFINE(HAVE_FAST,1,[define if you have the library FAST])
AC_SUBST(HAVE_FAST,"yes")],[AC_MSG_WARN(could not find the FAST lib. Will \
				try to build a degraded Benchs module, able \
				to benchs, but not to do any data fitting)])


dnl ###########################################################################
dnl ####[ search the BLACS Library ]
dnl ###########################################################################
#ACI_PACKAGE([BLACS],[the BLACS library: used by DIET samples and so \
#		necessary if they are compiled], 
#		[ldap_bind],
#		[-lldap -lldap!-L$with_OPENLDAP/lib!-I$with_OPENLDAP/include],
#		[ldap.h])

dnl ###########################################################################
dnl ####[ check for some programms ]
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


AC_SUBST(HAVE_FAST)
AC_SUBST(HAVE_CICHLID)
AC_SUBST(HAVE_STATISTICS)
AC_SUBST(OMNIORB_VERSION)
AC_SUBST(OMNIORB_HOME,"$OMNIORB_HOME")

AC_SUBST(IDLFLAGS)
AC_SUBST(prefix)
AC_SUBST(topsrcdir)
AC_LANG(C++)


dnl ###########################################################################
# Checks for header files.
dnl ###########################################################################
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h values.h])

dnl ###########################################################################
# Checks for typedefs, structures, and compiler characteristics.
dnl ###########################################################################
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

dnl ###########################################################################
# Checks for library functions.
dnl ###########################################################################
AC_FUNC_MALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_CHECK_FUNCS([gethostname gettimeofday select sqrt strcasecmp strdup strrchr \
                strstr])

dnl ###########################################################################
dnl #### check OS/Platform target
dnl ###########################################################################
case $host_os in
  linux*)
    AC_SUBST(OS_DIET, __linux__)
    case $host_cpu in
      *86*)
        AC_SUBST(ARCH_DIET, __x86__)
	;;
      *)
        ;;
    esac
    ;;
  *)
  ;;
esac

dnl ###########################################################################
dnl #### Compiling DIET modules with statistic services
dnl ###########################################################################
AM_CONDITIONAL(STATISTIC, test "$enable_stats" = yes) 


dnl ###########################################################################
dnl Create target output (Makefile(s) ...) 
dnl ###########################################################################
#include/DIET_config.h
AC_CONFIG_FILES([include/Makefile.inc
		config.mk
		Makefile
		src/Makefile
                src/CORBA/Makefile
                src/utils/Makefile
                src/SeD/Makefile
                src/agent/Makefile
                src/client/Makefile
                src/examples/BLAS/Makefile
                src/examples/dmat_manips/Makefile
                src/examples/file_transfer/Makefile
                src/examples/SCALAPACK/Makefile
		doc/Makefile
		doc/CS/Makefile
		doc/UM/Makefile])
AC_OUTPUT

dnl ###########################################################################
dnl #### Print general info on DIET 
dnl ###########################################################################
echo ""
echo "CONFIGURE successfully ended !!!"
echo "DIET configured with prefix: $prefix."
if test "x$HAVE_FAST" = "xyes"
then
echo "DIET configured with FAST in $with_FAST."
else
  echo "DIET configured without FAST."
fi
echo "DIET configured with omniORB $OMNIORB_VERSION in $OMNIORB_HOME."
if test "x$HAVE_Cichlid" = "xyes"
then
  echo "DIET configured with generation of communication logs for Cichlid."
else
  echo "DIET configured without Cichlid interface."
fi
if test "x$HAVE_STATISTICS" = "xyes"
then
  echo "DIET configured with generation of statistics logs."
else
  echo "DIET configured without any statistics."
fi

exit 0;
