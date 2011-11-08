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
        header=$(cat bin/scripts/copyright.txt)
        awk -v test="$header" '!/LICENSE/ { print $0 }; /\|LICENSE\|/ { print test }' $IN_FILE > $OUT_FILE-tmp
        mv $OUT_FILE-tmp $OUT_FILE
    ;;
  *.tex)
        header=$(sed 's/ */%/g' bin/scripts/copyright.txt)
        awk -v test="$header" '!/LICENSE/ { print $0 }; /\|LICENSE\|/ { print test }' $IN_FILE > $OUT_FILE-tmp
        mv $OUT_FILE-tmp $OUT_FILE
    ;;
  *.bib)
        header=$(sed 's/ */@comment %* /g' bin/scripts/copyright.txt)
        awk -v test="$header" '!/LICENSE/ { print $0 }; /\|LICENSE\|/ { print test }' $IN_FILE > $OUT_FILE-tmp
        mv $OUT_FILE-tmp $OUT_FILE

    ;;
  *.ps|*.eps|*.fig|*.gif|*.png|*.schema|*.ldif)
    exit 0
    ;;
  *)
esac

printf "Formatting $IN_FILE for distribution ... "

echo -n "done."

echo ""

