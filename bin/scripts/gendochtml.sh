#!/bin/sh

if [ $# -ne 1 ]; then
        echo 1>&2 Usage: $0 [gen,pub]
	echo "       gen: Generate the documentation"
	echo "       pub: Generate and publish the documentation" 
        exit -1
fi

cd ../../doc/UsersManual/
make
cp fig4latex2html/*.png UsersManual/fig/
cd UsersManual
latex2html -local_icons  -address "The DIET Team - `date`" UsersManual
cd UsersManual

# ---- Add favicon everywhere ----
for i in *.html
do 
    cp $i _tmp_.html
    sed s/"<\/HEAD>"/"<LINK REL=\"shortcut icon\" TYPE=\"image\/x-icon\" HREF=\"favicon.ico\"\/> \n<\/HEAD>"/g _tmp_.html > $i 
done 
rm _tmp_.html

# ---- Add XML GoDIET code ----
file1=`grep "Example XML input file" *.html | cut -f 1 -d ":" `
cp $file1  _tmp_.html 
sed s/"<TR><TD><\/TD><\/TR>"/"<TR><TD><DIV ALIGN="CENTER"><IMG SRC=\".\/xml_godiet.png\" ALT=\"Image XML GoDIET code\"\><\/DIV\><\/TD\><\/TR\>"/g   _tmp_.html > $file1
rm _tmp_.html

cp ../../fig4latex2html/favicon.ico . 
cp ../../fig4latex2html/xml_godiet.png . 

if [ $1 = "pub" ]; then
    scp * diet:public_html/UsersManual/.
fi
