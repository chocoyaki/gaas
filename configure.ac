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
#* Revision 1.55  2005/12/20 21:29:55  pfrauenk
#* CoRI defined symbols renamed
#*
#* Revision 1.54  2005/12/20 07:48:43  pfrauenk
#* CoRI functionality added : new option --enable-cori available to compile
#* with CoRI
#*
#*
#* Revision 1.53  2005/12/15 15:26:57  pfrauenk
#* CoRI adding flags for compiling by Peter Frauenkron
#*
#* Revision 1.52  2005/09/05 15:52:52  hdail
#* Addition of two configure options to control experimental development:
#* --enable-windowsched for MA-level window-based scheduling and
#* --enable-altpredict for an alternative prediction approach based on heuristic
#* estimates of 'distance'.
#*
#* Revision 1.51  2005/07/13 06:57:31  ecaron
#* Ready to version 2.0
#*
#* Revision 1.50  2005/07/12 19:42:41  hdail
#* Bugzilla Bug #12 corrected.  When building in a separate directory,
#* src/lib directory should only be copied over when --enable-JXTA-mode flag is
#* provided at configure time.
#*
#* Revision 1.49  2005/07/12 18:10:27  hdail
#* Remove excess warning messages about the dangers of not using fast.
#*
#* Revision 1.48  2005/06/08 15:27:08  ycaniou
#* Renamed JXTA-C in JXTA_C ('-' can not be used in a variable name)
#* Fixed the errors due to the use of AM_CONDITIONAL where '"' have to be used in
#*  the test part (for JXTA, JUXMEM and BATCH)
#*
#* Revision 1.47  2005/05/25 07:53:31  mjan
#* Bug fix for JuxMem support. Reported by Eddy. Thx!
#*
#* Revision 1.46  2005/05/18 14:18:09  mjan
#* Initial adding of JuxMem support inside DIET. dmat_manips examples tested without JuxMem and with JuxMem
#*
#* Revision 1.45  2005/05/16 15:27:32  alsu
#* adding plugin_example to the examples lists
#*
#* Revision 1.44  2005/05/02 14:28:50  ycaniou
#* The server lib needs appleseeds to be linked with, for all the examples
#* to compile without any problem.
#*
#* Revision 1.43  2005/04/27 01:53:05  ycaniou
#* Modified in order to really compile well (hope this to be the good one ;)
#*
#* Revision 1.42  2005/04/20 22:24:08  ycaniou
#* The tool now really checks if elbase.h and if libelagi.a are present
#*
#* Revision 1.41  2005/04/11 20:17:53  ycaniou
#* Correct the last commit :
#* - withdraw some conflicting lines
#* - add a USE_BATCH variable to allow the compilation if batch mode asked.. or not.
#*
#* Revision 1.40  2005/04/11 18:39:12  ycaniou
#* Include modifications to compile batch submission sections if elagi
#*  is installed, as well as an example.
#*
#* Revision 1.39  2005/04/08 13:02:42  hdail
#* The code for LogCentral has proven itself stable and it seems bug free.
#* Since no external libraries are required to compile in LogCentral, its now
#* going to be compiled in by default always ... its usage is easily controlled by
#* configuration file.
#*
#* Revision 1.38  2005/04/08 09:56:12  hdail
#* Removing unused Cichlid from DIET.
#*
#* Revision 1.37  2004/09/29 13:35:31  sdahan
#* Add the Multi-MAs feature.
#*
#* Revision 1.36  2004/09/17 14:08:26  rbolze
#* comment AC_FUNC_MALLOC and AC_FUNC_REMALLOC because of compile problem.
#*
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
#****************************************************************************#

define(DIET_version,        2.0)
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
AC_CONFIG_HEADERS([include/acDIET_config.h])
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
	       [SeD client],[Batch BLAS ScaLAPACK],maintainer-force)
    ACI_MODULE([Batch],
		[an example for calling functions through a batch scheduler via DIET],
		[examples Batch],,no)
    ACI_MODULE([BLAS],[an example for calling BLAS functions through DIET],
		[examples BLAS],,no)
    ACI_MODULE([ScaLAPACK],
		[an example for calling ScaLAPACK functions through DIET],
		[examples ScaLAPACK],,no)


dnl ###########################################################################
# Various compilation features
dnl ###########################################################################

AH_TEMPLATE(HAVE_STATISTICS, [Define the use of the statistics tools.])
AC_ARG_ENABLE(stats,
	      AC_HELP_STRING([--enable-stats],
			     [enable generation of statistics logs]),
	      [AC_DEFINE(HAVE_STATISTICS)][IDLFLAGS="$IDLFLAGS -DHAVE_STATISTICS"])

AH_TEMPLATE(HAVE_WINDOWSCHED, [Define the use of window scheduling features.])
AC_ARG_ENABLE(windowsched,
	      AC_HELP_STRING([--enable-windowsched],
			     [use experimental window scheduling]),
	      [AC_DEFINE(HAVE_WINDOWSCHED)][IDLFLAGS="$IDLFLAGS -DHAVE_WINDOWSCHED"])

AH_TEMPLATE(HAVE_ALTPREDICT, [Define the use of alternative perf. prediction.])
AC_ARG_ENABLE(altpredict,
	      AC_HELP_STRING([--enable-altpredict],
			     [use alternative performance prediction]),
	      [AC_DEFINE(HAVE_ALTPREDICT)][IDLFLAGS="$IDLFLAGS -DHAVE_ALTPREDICT"])

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
  JARDATA="lib/*.jar"
  USE_JXTA_MODE=yes
  NA_DETECT_JAVAC(required)
  NA_DETECT_JXTA
else
  JARDATA=""
fi

AM_CONDITIONAL(JXTA_MODE, [test "x$USE_JXTA_MODE" = "xyes"])
JXTA=$JXTA_MODE_TRUE

AH_TEMPLATE(HAVE_BATCH, [Define the use of the SeD Batch Submission.])
AC_ARG_ENABLE(Batch,
	AC_HELP_STRING([--enable-Batch],
			[enable submission of batch jobs]),
	[enable_batch=yes],[enable_batch=no])

dnl ##########################################
dnl Specific to CoRI
dnl ##########################################

AH_TEMPLATE([HAVE_CORI],[Define the use of CoRI - Collector of Ressource Information.])
AC_ARG_ENABLE(cori,
	      AC_HELP_STRING([--enable-cori],
			     [enable the Collector of Ressource Information, no scheduling possible. default=disabled]),
	      [enable_cori=yes],[enable_cori=no])

if test "x$enable_cori" = "xyes"
then
  AC_DEFINE(HAVE_CORI)
fi
AM_CONDITIONAL(USING_CORI,[test "x$enable_cori" = "xyes"])

dnl ##########################################
dnl Specific to JuxMem
dnl ##########################################

AH_TEMPLATE(HAVE_JUXMEM, [Define use of JuxMem by DIET.])
AC_ARG_ENABLE(JuxMem,
	      AC_HELP_STRING([--enable-JuxMem],
			     [to use JuxMem with DIET specify the path of JuxMem.]),
	      [enable_JuxMem=yes])

if test "x$enable_JuxMem" = "xyes"
then

AC_ARG_WITH(apr,
	AC_HELP_STRING([--with-apr], [location of APR to use (needed by JuxMem)]),
	[with_apr="$withval"],[enable_APR=no])

AC_ARG_WITH(jxta-c,
	AC_HELP_STRING([--with-jxta-c], [location of JXTA-C to use (needed by JuxMem)]),
	[with_jxta="$withval"],[enable_JXTA_C=no])

AC_ARG_WITH(juxmem,
	AC_HELP_STRING([--with-juxmem], [location of JuxMem to use]),
	[with_juxmem="$withval"],[enable_JUXMEM=no])

  AC_DEFINE(HAVE_JUXMEM)
  IDLFLAGS="$IDLFLAGS -DHAVE_JUXMEM"
  JUXMEM_CXXFLAGS=" -I$with_jxta/include -I$with_apr/include -I$with_juxmem/include"
  JUXMEM_LDFLAGS=" -L$with_apr/lib -L$with_jxta/lib -L$with_juxmem/lib -lapr-0 -laprutil-0 -ljpr -ljxta -ljuxmem"
else
  JUXMEM_CXXFLAGS=""
  JUXMEM_LDFLAGS=""
fi

  AC_SUBST(JUXMEM_CXXFLAGS)
  AC_SUBST(JUXMEM_LDFLAGS)

AM_CONDITIONAL(JUXMEM_MODE, [test "x$HAVE_JUXMEM" = "xyes"])


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
# Configuration for CoRI Plugin
dnl ###########################################################################

AC_CHECK_FILE([/proc/cpuinfo],[AC_DEFINE([CORI_HAVE_PROCCPU],[1],[blabla])],[CORI_HAVE_PROCCPU=no])
AC_CHECK_FILE([/proc/meminfo],[AC_DEFINE([CORI_HAVE_PROCMEM],[1],[blabla])],[CORI_HAVE_PROCMEM=no])

AC_CHECK_HEADER([sys/types.h],[AC_DEFINE([CORI_HAVE_SYS_TYPES],[1],[blabla])],[CORI_HAVE_SYS_TYPES=no])
AC_CHECK_HEADER([sys/sysctl.h],[AC_DEFINE([CORI_HAVE_SYS_SYSCTL],[1],[blabla])],[CORI_HAVE_SYS_SYSCTL=no])
AC_CHECK_HEADER([sys/sysinfo.h],[AC_DEFINE([CORI_HAVE_SYS_SYSINFO],[1],[blabla])],[CORI_HAVE_SYS_SYSINFO=no])

AC_CHECK_FUNCS([sysconf getloadavg sysctlbyname get_avphys_pages get_phys_pages getpagesize sysctl get_nprocs])

dnl ###########################################################################
# Search for the FAST Library
dnl ###########################################################################
AM_FAST(FAST_min_version,
	[HAVE_FAST=yes],
	[HAVE_FAST=no])

dnl ###########################################################################
# Search the ORB to use
dnl ###########################################################################
AC_OMNIORB_DIET(omniORB_min_version,,,,
		$proc_name,$proc_def,$plat_name,$plat_def,$os_v,
		HAVE_OMNIORB=yes,HAVE_OMNIORB=no)
if test "x$HAVE_OMNIORB" != "xyes"; then
  AC_MSG_ERROR([could not detect omniORB omniORB_min_version or higher])
fi
# In order omniORB to pass ACI_MODULES_VERIFY(DIET) which does not look
# for uppercase variable
HAVE_omniORB="yes"

# AC_TAO ???

dnl ###########################################################################
dnl ####[ search for the elagi and appleseeds Libraries ]
dnl ###########################################################################
if test "x$enable_batch" = "xyes"; then

AC_ARG_WITH(Elagi,
	[  --with-Elagi=DIR        location of elagi installation directory ],
	[ELAGI_DIR=${withval}
	ELAGI_INCLUDE_DIR="${C_INC}${withval}/include/elagi"],
        [echo "The Elagi installation dir is required: ./configure --enable-batch --with-Elagi=<dir>"
        exit 1])

	AC_CHECK_HEADER([${withval}/include/elagi/elbase.h],,
		  AC_MSG_ERROR([Could not detect elagi.h]))
	AC_CHECK_FILE([${withval}/lib/libelagi.a],,
		  AC_MSG_ERROR([Could not detect libelagi.a]))
	AC_CHECK_FILE([${withval}/include/elagi/elagi.make],,
		  AC_MSG_ERROR([Could not detect elagi.make]))

	AC_SUBST(ELAGI_DIR)

AC_ARG_WITH(Appleseeds,
	[  --with-Appleseeds=DIR        location of appleseeds installation directory ],
	[APPLESEEDS_DIR=${withval}
	APPLESEEDS_INCLUDE_DIR="${C_INC}${withval}/include/appleseeds"],
        [echo "The Appleseeds installation dir is required: ./configure --enable-batch --with-Appleseeds=<dir>"
        exit 1])

	AC_CHECK_HEADER([${withval}/include/appleseeds/appleseeds.h],,
		  AC_MSG_ERROR([Could not detect appleseeds.h]))
	AC_CHECK_FILE([${withval}/lib/libappleseeds.a],,
		  AC_MSG_ERROR([Could not detect libappleseeds.a]))
	AC_CHECK_FILE([${withval}/include/appleseeds/appleseeds.make],,
		  AC_MSG_ERROR([Could not detect appleseeds.make]))

	AC_SUBST(APPLESEEDS_DIR)


# Then we can enable all the flags (source and makefile ones)

	HAVE_BATCH="yes"
	HAVE_Batch="yes"

	AC_DEFINE(HAVE_BATCH)
# For information
# CFLAGS correspond to flags for the processor on which prog is built
# AM_CPPFLAGS: 
#	The contents of this variable are passed to every compilation
#       which invokes the C preprocessor; it is a list of arguments to the
#       preprocessor.  For instance, `-I' and `-D' options should be
#       listed here.
	AM_CPPFLAGS="-DHAVE_BATCH $AM_CPPFLAGS"

	# for Corba things (CORBA/idl/common_types.idl)
	IDLFLAGS="$IDLFLAGS -DHAVE_BATCH"
	CPPFLAGS_OMNIORB="-DHAVE_BATCH $CPPFLAGS_OMNIORB"
	CXXFLAGS_OMNIORB="-DHAVE_BATCH $CXXFLAGS_OMNIORB"
fi
# to 
AM_CONDITIONAL(USE_BATCH, [test "x$HAVE_BATCH" = "xyes"])


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
AC_SUBST(CXXFLAGS_OMNIORB)

AC_SUBST(IDLFLAGS)

AC_SUBST(JARDATA)

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
dnl AC_FUNC_MALLOC
dnl AC_FUNC_REALLOC
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
		 src/examples/Batch/Makefile
		 src/examples/BLAS/Makefile
		 src/examples/dmat_manips/Makefile
		 src/examples/file_transfer/Makefile
		 src/examples/plugin_example/Makefile
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

# Batch
printf " - Batch:           " | tee -a config.log
if test "x$enable_Batch" = "xyes"
then
  echo "yes" | tee -a config.log
else
  echo "no" | tee -a config.log
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
  printf "dmat_manips, file_transfer, plugin_example" | tee -a config.log
  if test "x$enable_batch" = "xyes"
  then
    printf ", Batch" | tee -a config.log
  fi
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

#CORI
printf " - CORI:            " | tee -a config.log
if test "x$enable_cori" = "xyes"
then
  echo "using CoRI " | tee -a config.log
else
  echo "disabled " | tee -a config.log
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

# JuxMem
printf " - JuxMem:          " | tee -a config.log
if test "x$enable_JuxMem" = "xyes"
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

# EXPERIMENTAL WINDOW SCHEDULING 
if test "x$enable_windowsched" = "xyes"
then
  echo " - Window Sched:    yes" | tee -a config.log
fi

# ALTERNATIVE PERFORMANCE PREDICTION
if test "x$enable_altpredict" = "xyes"
then
  echo " - Alt Predict:     yes" | tee -a config.log
fi

# PLATFORM
echo " - target platform: $target" | tee -a config.log

echo | tee -a config.log

echo "Please run make help to get compilation instructions."

exit 0
