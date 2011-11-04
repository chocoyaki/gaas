#!/bin/sh
#**
#*  @file  distrib_file.sh
#*  @brief  Clean up CVS fields in DIET files, when building a distribution.
#*  @author Philippe COMBES (Philippe.Combes@ens-lyon.fr)
#*  @section Licence 
#*    |LICENCE|



IN_FILE=$1
FILEDIR=`dirname $1`
FILEBASENAME=`basename $1`
OUT_FILE=$2

# Select comment syntax depending on the file extension
# Default is #* ... *#


case "$FILEBASENAME" in
  *.c|*.cc|*.h|*.hh|*.idl|*.java)
    OPC="/* " # open comments 
    CLC=" */" # close comments
    SEP_LINE_PAT="^/\\**\*/$"
    END_CVS_PAT="^\\**\*/$"
    ;;
  *.tex)
    OPC="%* " # open comments 
    CLC=" *%" # close comments
    SEP_LINE_PAT="^%\\**\*%$"
    END_CVS_PAT=$SEP_LINE_PAT
    ;;
  *.bib)
    OPC="@Comment %* " # open comments 
    CLC=" *%"          # close comments
    SEP_LINE_PAT="^@Comment %\\**\*%$"
    END_CVS_PAT=$SEP_LINE_PAT
    ;;
  *.ps|*.eps|*.fig|*.gif|*.png|*.schema|*.ldif)
    exit 0
    ;;
  *)
    OPC="#* " # open comments 
    CLC=" *#" # close comments
    SEP_LINE_PAT="^#\\**\*#$"
    END_CVS_PAT=$SEP_LINE_PAT
esac

printf "Formatting $IN_FILE for distribution ... "

# How to match the $LICENSE$ line ...
OPC_PAT=`echo "$OPC" | sed "s|\*|\\\\\*|g"`
CLC_PAT=`echo "$CLC" | sed "s|\*|\\\\\*|g"`
# for .tex and .bib files, the % must be doubled into the printf
OPC_PRNT=`echo "$OPC" | sed "s|%|%%|g"`
CLC_PRNT=`echo "$CLC" | sed "s|%|%%|g"`
LICENSE_PAT="^${OPC_PAT}\\\$LICENSE\\$ *${CLC_PAT}$"

# Are there CVS lines ?
if [ "$FILEBASENAME" = "Makefile.in" ]; then
  GREP_CVS_ID="grep '^$OPC_PAT .Id: Makefile.am,v .*\\\$$' $IN_FILE"
  if [ "`eval $GREP_CVS_ID`" = "" ]; then # it is not a generated Makefile.in
    GREP_CVS_ID="grep '^$OPC_PAT .Id: $FILEBASENAME,v .*\\\$$' $IN_FILE"
  fi
else
  GREP_CVS_ID="grep '^$OPC_PAT .Id: $FILEBASENAME,v .*\\\$$' $IN_FILE"
fi
if [ "`eval $GREP_CVS_ID`" = "" ]; then
  THERE_ARE_CVS_LINES=0
else
  THERE_ARE_CVS_LINES=1
fi

# Do not use complete Log spec so that CVS does not replace it in the script
# itself: $ is replaced by .
GREP_CONFIG_MK="grep '^#\*\\ .Log: config.mk.in,v .$' $IN_FILE"

# Loop on file lines until the header is processed.
# Then dump the file tail as is.
NO_LINE=0
NO_SEP_LINE=0
no_sep_line_file=/tmp/distrib_no_sep_line.$$
echo $NO_SEP_LINE > $no_sep_line_file
cat $IN_FILE | while read LINE
do
  
  NO_LINE=`expr $NO_LINE + 1`
  GOT_LICENSE_LINE=`echo "$LINE" | sed "s|${LICENSE_PAT}|yes|g"`
  if [ "$GOT_LICENSE_LINE" = "yes" ]
  then
    len=`printf "$OPC_PRNT" | wc -c`
    version_fmt=%-`expr 48 - $len`s
    printf "$OPC_PRNT This file is part of DIET $version_fmt$CLC_PRNT\n" \
           ${VERSION}. >> $OUT_FILE
    cat >> $OUT_FILE << EOF
$OPC                                                                        $CLC
$OPC Copyright ENS Lyon, INRIA, UCBL, SysFera (2000)                        $CLC
$OPC                                                                        $CLC
$OPC - Frederic.Desprez@ens-lyon.fr (Project Manager)                       $CLC
$OPC - Eddy.Caron@ens-lyon.fr (Technical Manager)                           $CLC
$OPC - Tech@sysfera.com (Maintainer and Technical Support)                  $CLC
$OPC                                                                        $CLC
$OPC This software is a computer program whose purpose is to provide an     $CLC
$OPC easy and transparent access to distributed and heterogeneous           $CLC
$OPC platforms.                                                             $CLC
$OPC                                                                        $CLC
$OPC                                                                        $CLC
$OPC This software is governed by the CeCILL license under French law and   $CLC
$OPC abiding by the rules of distribution of free software.  You can  use,  $CLC
$OPC modify and/ or redistribute the software under the terms of the CeCILL $CLC
$OPC license as circulated by CEA, CNRS and INRIA at the following URL      $CLC
$OPC "http://www.cecill.info".                                              $CLC
$OPC                                                                        $CLC
$OPC As a counterpart to the access to the source code and  rights to copy, $CLC
$OPC modify and redistribute granted by the license, users are provided     $CLC
$OPC only with a limited warranty  and the software's author,  the holder   $CLC
$OPC of the economic rights,  and the successive licensors  have only       $CLC
$OPC limited liability.                                                     $CLC
$OPC                                                                        $CLC
$OPC In this respect, the user's attention is drawn to the risks            $CLC
$OPC associated with loading,  using,  modifying and/or developing or       $CLC
$OPC reproducing the software by the user in light of its specific status   $CLC
$OPC of free software, that may mean  that it is complicated to             $CLC
$OPC manipulate, and  that  also therefore means  that it is reserved for   $CLC
$OPC developers and experienced professionals having in-depth computer      $CLC
$OPC knowledge. Users are therefore encouraged to load and test the         $CLC
$OPC software's suitability as regards their requirements in conditions     $CLC
$OPC enabling the security of their systems and/or data to be ensured and,  $CLC
$OPC more generally, to use and operate it in the same conditions as        $CLC
$OPC regards security.                                                      $CLC
$OPC                                                                        $CLC
$OPC The fact that you are presently reading this means that you have had   $CLC
$OPC knowledge of the CeCILL license and that you accept its terms.         $CLC
$OPC                                                                        $CLC
EOF

  elif [ "`echo \"$LINE\" | sed \"s|${SEP_LINE_PAT}|@@|g\"`" = "@@" ]
  then
    NO_SEP_LINE=`expr $NO_SEP_LINE + 1`
    printf '%s\n' "$LINE" >> $OUT_FILE
    if [ "$NO_SEP_LINE" = "2" ]; then
      if [ "$THERE_ARE_CVS_LINES" = "0" ]; then
	cat_tail=1
      else
        read LINE
        NO_LINE=`expr $NO_LINE + 1`
        # Remove CVS lines
        while [ "`echo \"$LINE\" | sed \"s|${END_CVS_PAT}|@@|g\"`" != "@@" ]
        do
          read LINE
          NO_LINE=`expr $NO_LINE + 1`
        done
        NO_SEP_LINE=`expr $NO_SEP_LINE + 1`
        cat_tail=0
      fi
    fi
    if [ $cat_tail ]; then
      NO_LINE=`expr $NO_LINE + 1`
      tail -n +$NO_LINE $IN_FILE >> $OUT_FILE
      echo $NO_SEP_LINE > $no_sep_line_file
      break;
    fi
  # The line must be kept
  else
    printf '%s\n' "$LINE" >> $OUT_FILE
  fi
done

echo -n "done."

if [ `cat $no_sep_line_file` -le 1 ]; then
  echo -n " [!!! WRONG FORMAT !!!] "
fi
echo ""
rm -f $no_sep_line_file

