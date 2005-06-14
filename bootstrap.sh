#!/bin/sh
# Run this to generate all the initial makefiles, etc.

######
###### Some defs
######

PKG_NAME="DIET"
# a list of all dirs containing possibly some macros
acmacrosdirs="${HOME}/share/aclocal ${PWD}/acmacros"
 
######
###### End of conf part
######
for dir in $acmacrosdirs ; do
   if test -d $dir ; then
   aclocalinclude="$aclocalinclude -I $dir";
   fi
done

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

echo "Autoregenerate package \`$PKG_NAME\` in directory \`$srcdir\`";

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\`" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

######
###### Some tests (borrowed from macros/autogen.sh from achtung)
######
# Changlog : 
# 27/5/00: Changed "to compile gnome" to "to compile $PKG_NAME"
# 27/5/00: removed gettext

DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf\` installed to compile $PKG_NAME."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}


# (grep "PROG_LIBTOOL" $srcdir/configure.ac >/dev/null) && {
#   (libtool --version) < /dev/null > /dev/null 2>&1 || {
#     echo
#     echo "**Error**: You must have \`libtool\` installed to compile $PKG_NAME."
#     echo "Get ftp://ftp.gnu.org/pub/gnu/libtool-1.4.tar.gz"
#     echo "(or a newer version if it is available)"
#     DIE=1
# 		NO_LIBTOOLIZE=yes
#   }
# }

#--------------------------------------------------------------------------
# libtool 1.4.2 or newer
#
LIBTOOL_WANTED_MAJOR=1
LIBTOOL_WANTED_MINOR=4
LIBTOOL_WANTED_PATCH=2
LIBTOOL_WANTED_VERSION=1.4.2

libtool=`which glibtool 2>/dev/null`
if test ! -x "$libtool"; then
    libtool=`which libtool`
fi
lt_pversion=`$libtool --version 2>/dev/null|head -1|sed -e 's/^[^0-9]*//g' -e 's/[- ].*//'`
if test -z "$lt_pversion"; then
    echo "bootstrap.sh: libtool not found."
    echo "You need libtool version $LIBTOOL_WANTED_VERSION or newer installed"
    echo "Get ftp://ftp.gnu.org/pub/gnu/libtool-1.4.tar.gz"
    echo "(or a newer version if it is available)"
    exit 1
fi
lt_version=`echo $lt_pversion` #|sed -e 's/\([a-z]*\)$/.\1/'`
IFS=.; set $lt_version; IFS=' '
lt_status="good"
if test "$1" = "$LIBTOOL_WANTED_MAJOR"; then
   if test "$2" -lt "$LIBTOOL_WANTED_MINOR"; then
      lt_status="bad"
   elif test ! -z "$LIBTOOL_WANTED_PATCH"; then

	    if test -n "$3"; then
	   if test "$3" -lt "$LIBTOOL_WANTED_PATCH"; then
lt_status="bad"
          fi


       fi
   fi
fi
if test $lt_status != "good"; then
  echo "bootstrap.sh: libtool version $lt_pversion found."
  echo "You need libtool version $LIBTOOL_WANTED_VERSION or newer installed"
  echo "Get ftp://ftp.gnu.org/pub/gnu/libtool-1.4.tar.gz"
  echo "(or a newer version if it is available)"
  exit 1
fi

echo "bootstrap.sh: libtool version $lt_version (ok)"


# -----------------------------------------------------------
AUTOMAKE_VERSION=`automake --version | grep automake | cut -d ')' -f 2 | sed -e 's/^[^0-9]*//g' -e 's/[- ].*//'`
AUTOMAKE_WANTED_MAJOR=1
AUTOMAKE_WANTED_MINOR=10

IFS=.; set $AUTOMAKE_VERSION; IFS=' '
lt_status="good"
if test "$1" = "$AUTOMAKE_WANTED_MAJOR"; then
   if test "$2" -lt "$AUTOMAKE_WANTED_MINOR"; then
      lt_status="bad"
    fi
else
   lt_status="bad"
fi
if test $lt_status != "good"; then
  echo
  echo "**Error**: You must have \`automake\` 1.7 or newer installed to compile $PKG_NAME."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.7.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
  NO_AUTOMAKE=yes
  exit 1
fi

echo "bootstrap.sh: automake version $AUTOMAKE_VERSION (ok)"

###### END of test part


######
###### Action part
######


#if test -z "$*"; then
#  echo "**Warning**: I am going to run \`configure\` with no arguments."
#  echo "If you wish to pass any to it, please specify them on the"
#  echo \`$0\`" command line."
#  echo
#fi

case $CC in
xlc )
  am_opt=--include-deps;;
esac

for coin in  $srcdir/configure.ac # `find $srcdir -name configure.ac -print`
do 
  dr=`dirname $coin`
  if test -f $dr/NO-AUTO-GEN; then
    echo skipping $dr -- flagged as no auto-gen
  else
    echo "************"
    echo "* Processing directory $dr"
    echo "************"
    
    macrodirs=`sed -n -e 's,AM_ACLOCAL_INCLUDE(\(.*\)),\1,gp' < $coin`
    ( cd $dr
      if grep "PROG_LIBTOOL" configure.ac >/dev/null; then
				if test -z "$NO_LIBTOOLIZE" ; then 
	  			echo "Running libtoolize --automake --force --copy in "`pwd`"..."
	  			libtoolize --automake --force --copy
				fi
      fi
      echo "Running aclocal $aclocalinclude..."
      aclocal $aclocalinclude 
      if grep "CONFIG_HEADER" configure.ac >/dev/null; then
				echo "Running autoheader in "`pwd`"..."
				autoheader
      fi
      echo "Running automake --add-missing --copy $am_opt ..."
      automake --add-missing --copy $am_opt
      echo "Running autoconf..."
      autoconf
    )
  fi
done

#conf_flags="--enable-maintainer-mode --enable-compile-warnings"
#--enable-compile-warnings=minimum"

#echo "rebuilding the api documentation... "
#doxygen && echo "Done with the rebuilding of the api documentation." && cd .. || exit 1


#if test x$NOCONFIGURE = x; then
#  echo "************"
#  echo "* Running $srcdir/configure $conf_flags $@ ..."
#  echo "************"
#
#  date>$srcdir/autogen-timestamp
#  $srcdir/configure $conf_flags "$@" || exit 1
#else
#  echo Skipping configure process.
#fi

#echo Now type \`make\` to compile $PKG_NAME


