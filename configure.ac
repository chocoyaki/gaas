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
#* Revision 1.35  2004/09/13 14:53:10  sdahan
#* changes the ways used to looking for the omniORB library.
#* the configure is now able to find omniORB on bsd system.
#*
#* Revision 1.34  2004/08/27 16:28:18  ctedesch
#* - Use of the asynchronous PIF scheme for propagation of the requests inside
#* the DIET J multi-hierarchy
#* - DIET/JXTA -> DIET J
#* - Change the JXTA examples scripts to build a whole multi-hierarchy
#*
#* Revision 1.33  2004/07/09 19:51:57  ctedesch
#* - javac and javah detected instead of java
#* - JXTA_LIB automatically set before compilation
#* - JXTA libraries copied into install_dir/lib
#* - DIET license added in the java files
#* - JXTA loaders scripts modified: set JXTA_LIB for execution
#* - update doc
#*
#* Revision 1.32  2004/07/08 20:04:11  rbolze
#* doc/ and doc/UsersManual is for non maintainer-mode
#*
#* Revision 1.31  2004/07/07 18:17:55  rbolze
#* ready to 1.1 version
#*
#* Revision 1.30  2004/06/11 15:45:39  ctedesch
#* add DIET/JXTA
#*
#* Revision 1.29  2004/04/19 08:52:42  rbolze
#* add a line to informe you if you have or not configure with the logservice option
#*
#* Revision 1.28  2004/03/01 18:21:45  rbolze
#* add the option --enable-logservice
#*
#* Revision 1.27  2004/02/27 16:25:55  ecaron
#* Change bug report to http://graal.ens-lyon.fr/bugzilla
#*
#* Revision 1.26  2004/02/11 07:10:09  ecaron
#* Tag version 1.0
#*
#* Revision 1.25  2003/12/17 16:11:41  ecaron
#* Change "tutorial" directory to "Tutorial"
#*
#* Revision 1.24  2003/12/15 17:24:52  pcombes
#* Add the tutorial and the solutions to the compilation chain.
#*
#* Revision 1.23  2003/09/26 14:04:12  pcombes
#* DIET 0.9. Local FAST platform: chmod +x on the scripts. Module examples
#* is forced in maintainer mode, but disabled by default in user mode.
#*
#* Revision 1.22  2003/09/22 21:05:57  pcombes
#* Add an example of a local platform with LDAP + NWS + CORBA Name Server.
#*
#* Revision 1.21  2003/09/09 12:54:46  pcombes
#* Reorganization of doc.
#*
#* Revision 1.20  2003/08/22 13:03:23  pcombes
#* FAST not found is not right when --without-FAST is specified.
#*
#* Revision 1.19  2003/08/09 17:34:26  pcombes
#* Print the version of FAST at the end of the configuration.
#*
#* Revision 1.18  2003/08/01 19:01:22  pcombes
#* Clean up omniORB and FAST detection - particularly for versions.
#*
#* Revision 1.17  2003/07/04 09:47:49  pcombes
#* Check for -Wno-strict-aliasing option (to be improved with AC macros).
#*
#* Revision 1.15  2003/06/24 15:18:30  pcombes
#* --enable-* options: some HAVE_* variables are useless.
#*
#* Revision 1.13  2003/06/23 13:13:27  pcombes
#* Re-Submit BLAS and ScaLAPACK detection to "enable" options.
#* Change configuration summary (more complete)
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
#****************************************************************************#

define(DIET_version,        1.1)
define(omniORB_min_version, 3)
define(FAST_min_version,    0.4.3)



dnl ##########################################################################
dnl #### Process this file with autoconf to produce a configure script.
dnl ##########################################################################
AC_PREREQ([2.55])
ACI_PREREQ(2003.01.29)
AC_INIT(DIET, DIET_version, http://graal.ens-lyon.fr/bugzilla)
AC_CONFIG_SRCDIR([include/DIET_data.h])
AC_REVISION($Revision$)
AC_CONFIG_HEADERS([include/DIET_config.h])
AC_PREFIX_DEFAULT(`pwd`/install)


dnl ##########################################################################
dnl #### Create templates for config.h.in header.
dnl ##########################################################################
AH_TOP([#ifndef _DIET_CONFIG_H_
#define _DIET_CONFIG_H_])
AH_BOTTOM([#endif // _DIET_CONFIG_H_])

AM_MAINTAINER_MODE
if test "x$enable_maintainer_mode" = "xyes" -a ! -f "$srcdir/bootstrap.sh"
then
  echo
  echo "**********************************************************************"
  echo "* ERROR: $0: this version of DIET cannot be configured in maintainer mode."
  echo "**********************************************************************"
  exit 1
fi

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE[]

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
# The examples are forced for maintainers, disabled by default for users.
    ACI_MODULE([examples],[basic DIET examples],
	       [SeD client],[BLAS ScaLAPACK],maintainer-force)
      ACI_MODULE([BLAS],[an example for calling BLAS functions through DIET],
	         [examples BLAS],,no)
      ACI_MODULE([ScaLAPACK],
	         [an example for calling ScaLAPACK functions through DIET],
	         [examples ScaLAPACK],,no)


dnl ###########################################################################
# Various compilation features
dnl ###########################################################################

AH_TEMPLATE(HAVE_CICHLID,
	    [Define the use of Cichlid and the communication tools.])
AC_ARG_ENABLE(Cichlid,
	      AC_HELP_STRING([--enable-Cichlid],
	                 [enable generation of communication logs for Cichlid]),
	      AC_DEFINE(HAVE_CICHLID,,
		        [Define the use of Cichlid and the communication tools.]))

AH_TEMPLATE(HAVE_STATISTICS, [Define the use of the statistics tools.])
AC_ARG_ENABLE(stats,
	      AC_HELP_STRING([--enable-stats],
			     [enable generation of statistics logs]),
	      [AC_DEFINE(HAVE_STATISTICS)][IDLFLAGS="$IDLFLAGS -DHAVE_STATISTICS"])

AH_TEMPLATE(HAVE_MULTI_MA, [Define the use of a multi MA architecture.])
AC_ARG_ENABLE(multi-MA,
	      AC_HELP_STRING([--enable-multi-MA],
			     [In progress. You should use the JXTA version 
for this feature]),
	      [AC_DEFINE(HAVE_MULTI_MA)][IDLFLAGS="$IDLFLAGS -DHAVE_MULTI_MA"])

AH_TEMPLATE([HAVE_JXTA], [Define use of DIET/JXTA architecture.])
AC_ARG_ENABLE(JXTA-mode,
    	AC_HELP_STRING([--enable-JXTA-mode],
			[enable DIET/J architecture]),
	[enable_JXTA=yes],
	[USE_JXTA_MODE=no]
	)

if test "x$enable_JXTA" = "xyes"
then
  AC_DEFINE(HAVE_JXTA)
  IDLFLAGS="$IDLFLAGS -DHAVE_JXTA"
  USE_JXTA_MODE=yes
  NA_DETECT_JAVAC(required)
  NA_DETECT_JXTA
fi

AM_CONDITIONAL(JXTA_MODE, [test $USE_JXTA_MODE = yes])
JXTA=$JXTA_MODE_TRUE


AH_TEMPLATE(HAVE_LOGSERVICE, [Define the use of LogService.])
AC_ARG_ENABLE(logservice,
	      AC_HELP_STRING([--enable-logservice],
			     [enable monitoring through LogService]),
	      [AC_DEFINE(HAVE_LOGSERVICE)][IDLFLAGS="$IDLFLAGS -DHAVE_LOGSERVICE"])

dnl ###########################################################################
dnl Load anything under acmacros/*.m4
dnl ###########################################################################
ACLOCAL="$ACLOCAL -I acmacros"


dnl ###########################################################################
# Checks for programs.
dnl ###########################################################################

dnl C compiler
AC_PROG_CXX
AC_LANG_CPLUSPLUS
dnl Compiler tools
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

dnl AC_PROG_LIBTOOL sets $CC:
dnl we now can test some warning options with CC and CXX

echo "int main(int argc, char* argv[]) {return 0;}" > conftest.c

AC_CHECK_OPTIONS($CC, [conftest.c -o conftest], -Wno-strict-aliasing,
		 [AC_SUBST(CFLAG_NO_STRICT_ALIASING,
			   "-Wno-strict-aliasing")],)
AC_CHECK_OPTIONS($CXX, [conftest.c -o conftest], -Wno-strict-aliasing,
 	         [AC_SUBST(CXXFLAG_NO_STRICT_ALIASING,
			   "-Wno-strict-aliasing")],)
AC_CHECK_OPTIONS($CXX, [conftest.c -o conftest], -Wno-ctor-dtor-privacy,
		 [AC_SUBST(CXXFLAG_NO_CTOR_DTOR_PRIVACY,
			   "-Wno-ctor-dtor-privacy")])

rm -f conftest.c conftest

dnl ###########################################################################
# Check for the pthread library
dnl ###########################################################################
dnl Detected in AC_OMNIORB_DIET :
dnl AC_CHECK_LIB([pthread], [pthread_attr_getdetachstate])

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
AM_FAST(FAST_min_version,
	[HAVE_FAST=yes],
	[HAVE_FAST=no
	 AC_MSG_WARN(Use of FAST library is disabled - either by you or)
         AC_MSG_WARN(because it was not found: all performance evaluations)
	 AC_MSG_WARN(will be set to infinity.)])

dnl ###########################################################################
dnl ####[ search for the BLAS Library ]
dnl ###########################################################################
if test "x$enable_BLAS" = "xyes"; then
  ACI_PACKAGE([BLAS],[Basic Linear Algebric Subroutines],[dgemm_],
              [-lcblas -lblas -lblas2 -latlas!-lblas!-llapack!-lm!-lblas!\
               -lcblas!-llapack!-lm!-L/usr/lib/atlas ],,,:)
fi

dnl ###########################################################################
dnl ####[ search for the ScaLAPACK Library ]
dnl ###########################################################################
if test "x$enable_ScaLAPACK" = "xyes"; then
  ACI_PACKAGE([ScaLAPACK],[a parallel version of the LAPACK library],[pdgemm_],
	      [-latlas],,,:)
fi

dnl TO DO: PBLAS, MPI, etc.

dnl ###########################################################################
dnl ####[ search for the BLACS Library ]
dnl ###########################################################################
dnl ACI_PACKAGE([BLACS],[the BLACS library: used by DIET samples and so \
dnl 		necessary if they are compiled], 
dnl 		[ldap_bind],
dnl 		[-lldap -lldap!-L$with_OPENLDAP/lib!-I$with_OPENLDAP/include],
dnl 		[ldap.h])

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
if test "x$HAVE_BLAS" = "xyes" -o "x$HAVE_ScaLAPACK" = "xyes" ; then
  AC_FUNC_STRTOD # used in BLAS and ScaLAPACK clients
fi
AC_CHECK_FUNCS([gethostname gettimeofday strdup strrchr strtok_r])


dnl ###########################################################################
dnl Create target output (Makefile(s) ...) 
dnl ###########################################################################
AC_CONFIG_FILES([config.mk
		 include/Makefile.inc
		 Makefile
		 src/Makefile
		 src/CORBA/Makefile
		 src/CORBA/idl/Makefile
		 src/utils/Makefile
		 src/SeD/Makefile
		 src/agent/Makefile
		 src/client/Makefile
		 src/examples/Makefile
		 src/examples/before.mk
		 src/examples/BLAS/Makefile
		 src/examples/dmat_manips/Makefile
		 src/examples/file_transfer/Makefile
		 src/examples/JXTA/Makefile
		 src/examples/ScaLAPACK/Makefile
		 src/examples/scalars/Makefile])
AC_CONFIG_FILES([src/examples/bin/local_corba_ns.sh
		 src/examples/bin/local_ldap.sh
		 src/examples/bin/local_nws.sh
		 src/examples/bin/local_platform.sh],
		[chmod +x src/examples/bin/*.sh])
AC_CONFIG_FILES([doc/Makefile
		doc/UsersManual/Makefile])
if test "x$enable_maintainer_mode" = "xyes"
then
  AC_CONFIG_FILES([doc/ProgrammersGuide/Makefile
		doc/Tutorial/Makefile
		doc/Tutorial/solutions/exercise2/Makefile
		doc/Tutorial/solutions/exercise3/Makefile])
fi

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
  printf ", scalars" | tee -a config.log
  echo | tee -a config.log
fi

# FAST
printf " - FAST:            " | tee -a config.log
if test "x$HAVE_FAST" = "xyes"
then
  echo "$FAST_VERSION (found $FAST_CONFIG)" | tee -a config.log
else
  echo "disabled" | tee -a config.log
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
if test "x$enable_multi_MA" = "xyes"
then
  echo "yes" | tee -a config.log
else
  echo "no" | tee -a config.log
fi

# DIET J
printf " - DIET J:          " | tee -a config.log
if test "x$enable_JXTA" = "xyes"
then
  echo "yes" | tee -a config.log
else
  echo "no" | tee -a config.log
fi

# LOGSERVICE
printf " - LogService:      " | tee -a config.log
if test "x$enable_logservice" = "xyes"
then
  echo "yes" | tee -a config.log
else
  echo "no" | tee -a config.log
fi

# STATISTICS
if test "x$enable_stats" = "xyes"
then
  echo " - statistics log:  yes" | tee -a config.log
fi

# PLATFORM
echo " - target platform: $target" | tee -a config.log

echo | tee -a config.log

echo "Please run make help to get compilation instructions."

exit 0
