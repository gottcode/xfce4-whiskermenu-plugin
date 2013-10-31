#!/bin/sh


BASEDIR=../panel-plugin
WDIR=`pwd`


echo -n 'Preparing desktop file...'
cd ${BASEDIR}
rm -f whiskermenu.desktop.in.h
rm -f whiskermenu.desktop.in
cp whiskermenu.desktop whiskermenu.desktop.in
sed -e '/Name\[/ d' \
	-e '/Comment\[/ d' \
	-e 's/Name/_Name/' \
	-e 's/Comment/_Comment/' \
	-i whiskermenu.desktop.in
intltool-extract --quiet --type=gettext/ini whiskermenu.desktop.in
cd ${WDIR}
echo ' DONE'


echo -n 'Extracting messages...'
# Sort alphabetically to match cmake
xgettext --from-code=UTF-8 --c++ --keyword=_ --keyword=N_:1 --sort-output \
	--package-name='Whisker Menu' --copyright-holder='Graeme Gott' \
	--output=xfce4-whiskermenu-plugin.pot ${BASEDIR}/*.cpp ${BASEDIR}/*.h
echo ' DONE'


echo -n 'Merging translations...'
for POFILE in *.po;
do
	echo -n " $POFILE"
	msgmerge --quiet --update --backup=none $POFILE xfce4-whiskermenu-plugin.pot
done
echo ' DONE'


echo -n 'Merging desktop file translations...'
cd ${BASEDIR}
intltool-merge --quiet --desktop-style ${WDIR} whiskermenu.desktop.in whiskermenu.desktop
rm -f whiskermenu.desktop.in.h
rm -f whiskermenu.desktop.in
echo ' DONE'
