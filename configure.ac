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
AC_INIT(diet, 0.7, diet-dev@ens-lyon.fr)
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


AC_ARG_ENABLE(multi-MA,[  --enable-multi-MA       enable multi MA architecture], desireMA=true,desireMA=false)
if test "x$desireMA" = "xtrue"
	then
	echo HAVE_MULTI_MA
	AC_DEFINE(HAVE_MULTI_MA)
fi

dnl ###########################################################################
# Initialize the test suite and build position independent wrappers.
dnl ###########################################################################
#AC_CONFIG_TESTDIR([tests])
#AC_CONFIG_FILES([tests_auto/Makefile tests_auto/atlocal])

dnl ###########################################################################
dnl DIET modules listing. 
dnl ###########################################################################
ACI_MODULE([diet],[The middleware you are trying to use ;)],[omniORB] [FAST],[agent client SeD],,[CORBA  utils])
  ACI_MODULE([agent],[Master and Local agent;],)
  ACI_MODULE([client],[client for master agent;],)
  ACI_MODULE([SeD],[basic diet service server;],)
  ACI_MODULE([examples],[basic diet service server;],)

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

dnl ###########################################################################
dnl ####[ search omniORB lib and bin ]
dnl ###########################################################################
AC_MSG_NOTICE([ ------ search omniorb lib, bin and tools ------ ])
AC_CHECK_FILE($OMNIORB_HOME, env_flag=true, env_flag=false)
OMNIORB_VERSION="undefined"
if test "x$env_flag" = "xtrue"
  then
  AC_MSG_NOTICE([OMNIORB_HOME is defined!])
  AC_CHECK_FILE($with_omniORB, omniorb_arg=true, omnbiorb_arg=false)
  if test "x$omniorb_arg" = "xtrue"
    then
    AC_MSG_NOTICE([configure argument --with-omniORB is also defined!\
                  so we use it for testing before OMNIORB_HOME])
    ACI_PACKAGE([omniORB],[omniORB],
    		[omni_thread_wrapper],
                [-lomnithread!-lomniDynamic4!-lomniORB4 \
                -lomnithread!-lomniDynamic3!-lomniORB3!-ltcpwrapGK],
		[omnithread.h], foundFirst=true)
    if test "x$founfFirst" = "xfalse"
      then
      AC_MSG_NOTICE([configure argument --with-omniORB is not valid ???.\
                    Try to use OMNIORB_HOME.])
        $with_omniORB=$OMNIORB_HOME
	ACI_PACKAGE([omniORB],[omniORB],
    	            [omni_thread_wrapper],
  		    [-lomnithread!-lomniDynamic4!-lomniORB4 \
                    -lomnithread!-lomniDynamic3!-lomniORB3!-ltcpwrapGK],
		    [omnithread.h], found=true)
    fi
    if test "x$founfFirst" = "xtrue"
      then
      OMNIORB_HOME=$with_omniORB
      found=true
    fi
  fi
fi
if test "x$env_flag" = "xfalse"
  then
  AC_MSG_NOTICE([OMNIORB_HOME is not defined! Try to find omniORB with \
              classic path and configure arg --with-OMNIORB.])
  ACI_PACKAGE([omniORB],[omniORB],
              [omni_thread_wrapper],
              [-lomnithread!-lomniDynamic4!-lomniORB4 \
              -lomnithread!-lomniDynamic3!-lomniORB3!-ltcpwrapGK],
	      [omnithread.h], found=true && OMNIORB_HOME=$with_omniORB)
fi

AC_MSG_NOTICE([Testing omniidl tool ...])
if test "x$found" = "xtrue"
  then
  AC_PATH_PROG(IDL_COMPILER,omniidl,idl_not_found,[$with_omniORB/bin])
  IDLFLAGS="-bcxx -Wba -Wbtp"
  if test "x$IDL" = "xidl_not_found"
    then
    AC_MSG_NOTICE([unable to find omniidl tool])
  fi
  AC_CHECK_HEADER($with_omniORB/include/omniORB4/omniutilities.h, 
                  AC_DEFINE(__OMNIORB4__, [], 
                  [OmniORB detected version]), omniVersion = false)
  if test "x$omniVersion" = "xfalse"
    then
    AC_CHECK_HEADER($with_omniORB/include/omniORB3/omniutilities.h, 
                    AC_DEFINE(__OMNIORB3__, [], 
                    [OmniORB detected version]), omniVersion3 = false)
    if test "x$omniVersion3" != "xfalse"
      then
      AC_MSG_NOTICE([omniORB detected version : 3])
                    OMNIORB_VERSION=3
    else
      AC_MSG_NOTICE([omniORB detected version : undefined])
    fi
  else
    AC_MSG_NOTICE([omniORB detected version : 4])
    OMNIORB_VERSION=4
  fi
fi


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
ACI_MODULES_VERIFY([diet]) 
AC_SUBST(MODULES_diet)
AC_SUBST(MODULES_agent)
AC_SUBST(MODULES_SeD)
AC_SUBST(MODULES_client)
AC_SUBST(MODULES_examples)
AC_SUBST(MODULES_doc)

AC_SUBST(DIST_MODULES_diet)
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
#AC_SUBST(LIB_BLAS)

AC_SUBST(IDLFLAGS)
AC_SUBST(prefix)
AC_SUBST(topsrcdir)
AC_LANG(C++)

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_attr_getdetachstate])

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
dnl #### Prinf info on compilers and args
dnl ###########################################################################
echo " 

Package ${PACKAGE} configuration : 
	compiler : 
		CC=${CC} 
		CXX=${CXX} 
		GCC=${GCC} 
		CPP=${CPP} 
	CFLAGS=${CFLAGS} 
	CXXFLAGS=${CFLAGS} 
	CPPFLAGS=${CFLAGS} 
	IDLFLAGS=${IDLFLAGS} 
	LDFLAGS=${CFLAGS} 
	LIBS=${LIBS} 
	FLIBS=${FLIBS} 
	DEP_LIBS=.... 
	"

dnl ###########################################################################
dnl #### Print info on modules and packages
dnl ###########################################################################
case $host_os in
  *)
    ACI_MODULES_SUMMARY
#   ACI_MODULES_SUMMARY_FANCY([diet])
  ;;
esac


dnl ###########################################################################
dnl #### Print general info on DIET 
dnl ###########################################################################
echo ""
echo "CONFIGURE successfully ended !!!"
echo "DIET configured with prefix: $prefix."
if test "x$HAVE_FAST" = "xyes"
then
  echo "DIET configured with FAST in $(with-FAST)."
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
