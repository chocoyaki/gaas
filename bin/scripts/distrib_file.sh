#!/bin/sh
#****************************************************************************#
#* Clean up CVS fields in DIET files, when building a distribution.         *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#
#* $Id$
#* $Log$
#* Revision 1.1  2003/09/22 21:06:05  pcombes
#* script becomes scripts.
#*
#* Revision 1.8  2003/09/09 12:55:49  pcombes
#* New distrib generation.
#*
#* Revision 1.7  2003/06/23 13:18:05  pcombes
#* Fix bug for grepping the CVS Id line
#*
#* Revision 1.6  2003/06/16 18:09:11  pcombes
#* Fix bug for the examples of configuration files.
#*
#* Revision 1.5  2003/06/03 18:28:14  pcombes
#* Dump the full license in place of the $LICENSE$ line.
#*
#* Revision 1.4  2003/04/14 14:32:36  pcombes
#* 2nd attempt to fix bug introduced by CVS field replacement.
#*
#* Revision 1.2  2003/04/10 13:56:06  pcombes
#* Simpler, thanks to new CS: CVS Id and Log regrouped.
#*
#* Revision 1.1  2003/02/14 15:09:51  pcombes
#* Tool for forthcoming make dist : clean CVS fields in files.
#* Tested on Linux, Solaris, FreeBSD and MacOSX.
#****************************************************************************#


ORG_FILE=$1
FILEDIR=`dirname $1`
FILEBASENAME=`basename $1`

shift

# Test if $ORG_FILE has to be processed
for i in $*  # $i can be a regular expression ...
do
  if [ "`echo $ORG_FILE | grep $i`" != "" ]; then
    exit 0;
  fi
done

# If $ORG_FILE is a directory, process its contents and exit
if [ -d "$ORG_FILE" ]; then
  for f in `ls -A $ORG_FILE`; do
    shift
    $0 $ORG_FILE/$f $*
    status=$?
    if [ $status -ne 0 ]; then exit $status; fi
  done
  exit 0;
fi
  

# Select comment syntax depending on the file extension
# Default is #* ... *#


case "$FILEBASENAME" in
  *.c|*.cc|*.h|*.hh|*.idl)
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
  *.ps|*.eps|*.fig|*.gif)
    exit 0
    ;;
  *)
    OPC="#* " # open comments 
    CLC=" *#" # close comments
    SEP_LINE_PAT="^#\\**\*#$"
    END_CVS_PAT=$SEP_LINE_PAT
esac

printf "Formatting $ORG_FILE for distribution ... "

IN_FILE=$ORG_FILE.org
OUT_FILE=$ORG_FILE
# Save original file to return in the same state after the distribution is built
mv -f $ORG_FILE $IN_FILE

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
else
  GREP_CVS_ID="grep '^$OPC_PAT .Id: $FILEBASENAME,v .*\\\$$' $IN_FILE"
fi
if [ "`eval $GREP_CVS_ID`" = "" ]; then
  NO_CVS_LINE=1
else
  NO_CVS_LINE=""
fi

# Do not use complete Log spec so that CVS does not replace it in the script
# itself: $ is replaced by .
GREP_CONFIG_MK="grep '^#\*\\ .Log: config.mk.in,v .$' $IN_FILE"

# Loop on file lines until the header is processed.
# Then dump the file tail as is.
NO_LINE=0
NO_SEP_LINE=0
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
$OPC Copyright (C) 2000-2003 ENS Lyon, LIFC, INSA and INRIA,                $CLC
$OPC                         all rights reserved.                           $CLC
$OPC                                                                        $CLC
$OPC Since DIET is open source, free software, you are free to use, modify, $CLC
$OPC and distribute the DIET source code and object code produced from the  $CLC
$OPC source, as long as you include this copyright statement along with     $CLC
$OPC code built using DIET.                                                 $CLC
$OPC                                                                        $CLC
$OPC Redistribution and use in source and binary forms, with or without     $CLC
$OPC modification, are permitted provided that the following conditions     $CLC
$OPC are met.                                                               $CLC
$OPC                                                                        $CLC
$OPC Redistributions of source code must retain the copyright notice below  $CLC
$OPC this list of conditions and the following disclaimer. Redistributions  $CLC
$OPC in binary form must reproduce the copyright notice below, this list    $CLC
$OPC of conditions and the following disclaimer in the documentation        $CLC
$OPC and/or other materials provided with the distribution. Neither the     $CLC
$OPC name of ENS Lyon nor the names of its contributors (LIFC, INSA Lyon,   $CLC
$OPC INRIA) may be used to endorse or promote products derived from this    $CLC
$OPC software without specific prior written permission.                    $CLC
$OPC                                                                        $CLC
$OPC THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    $CLC
$OPC "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      $CLC
$OPC LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS      $CLC
$OPC FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE         $CLC
$OPC REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,            $CLC
$OPC INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,   $CLC
$OPC BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES ;      $CLC
$OPC LOSS OF USE, DATA, OR PROFITS ; OR BUSINESS INTERRUPTION) HOWEVER      $CLC
$OPC CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT     $CLC
$OPC LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY  $CLC
$OPC WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE            $CLC
$OPC POSSIBILITY OF SUCH DAMAGE.                                            $CLC
$OPC                                                                        $CLC
EOF
    
  else if [ "`echo \"$LINE\" | sed \"s|${SEP_LINE_PAT}|@@|g\"`" = "@@" ]
  then
    NO_SEP_LINE=`expr $NO_SEP_LINE + 1`
    printf '%s\n' "$LINE" >> $OUT_FILE
    if [ "$NO_SEP_LINE" = "2" -a "$NO_CVS_LINE" = "" ]; then
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
    # Is it a Makefile.in that includes config.mk ???
    if [ "$FILEBASENAME" = "Makefile.in" -a \
         "`eval $GREP_CONFIG_MK`" != ""  -a "$NO_SEP_LINE" = "3" ]; then
      while [ "$NO_SEP_LINE" -lt "6" ]
      do
	read LINE
	NO_LINE=`expr $NO_LINE + 1`
	if [ "`echo \"$LINE\" | sed \"s|${SEP_LINE_PAT}|@@|g\"`" = "@@" ]
	then
	  NO_SEP_LINE=`expr $NO_SEP_LINE + 1`
	fi
      done
    fi
    if [ $cat_tail ]; then
      NO_LINE=`expr $NO_LINE + 1`
      tail +$NO_LINE $IN_FILE >> $OUT_FILE
      break;
    fi
  # The line must be kept
  else
    printf '%s\n' "$LINE" >> $OUT_FILE
  fi fi
done


rm -f $IN_FILE

echo "done."