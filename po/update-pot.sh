#!/bin/sh


BASEDIR=../panel-plugin
WDIR=`pwd`


echo -n 'Preparing desktop file...'
cd ${BASEDIR}
rm -f whiskermenu.desktop.in
cp whiskermenu.desktop whiskermenu.desktop.in
sed -e '/Name\[/ d' \
	-e '/Comment\[/ d' \
	-e '/Icon/ d' \
	-i whiskermenu.desktop.in
cd ${WDIR}
echo ' DONE'


echo -n 'Extracting messages...'
# Sort alphabetically to match cmake
xgettext --from-code=UTF-8 --c++ --keyword=_ --sort-output \
	--package-name='Whisker Menu' --copyright-holder='Graeme Gott' \
	--output=xfce4-whiskermenu-plugin.pot ${BASEDIR}/*.cpp ${BASEDIR}/*.h
xgettext --from-code=UTF-8 --join-existing --sort-output \
	--package-name='Whisker Menu' --copyright-holder='Graeme Gott' \
	--output=xfce4-whiskermenu-plugin.pot ${BASEDIR}/*.desktop.in
echo ' DONE'


echo -n 'Cleaning up...'
cd ${BASEDIR}
rm -f whiskermenu.desktop.in
echo ' DONE'
