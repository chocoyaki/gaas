#!/bin/sh
# This script should be run from within your build directory.
# It removes all paths before libraries dependencies in binary
# and dylib files.


FILES_BIN=`find . -type f \( -perm -u=x -o -perm -g=x -o -perm -o=x \) | grep -v -e CMakeFiles -e Cmake -e doc/`
FILES_DYLIB=`find . -name *.dylib`
FILES="${FILES_BIN} ${FILES_DYLIB}"
FILES=`echo $FILES |sort | uniq`
for i in $FILES; do
    echo "## Dealing with: $i"

    FILE=$i

    # get list of libraries present in DIET binaries and libraries
    LIBS=`otool -l $FILE | grep -v -e segname -e sectname | grep name | awk '{print $2}'`

    # remove path to libraries
    CHANGE=""
    for l in $LIBS; do
        CHANGE="-change $l `basename $l` $CHANGE"
    done
    install_name_tool $CHANGE $FILE
done

echo "### All done! ###"