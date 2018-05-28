#!/bin/sh


BASEDIR=../panel-plugin
WDIR=`pwd`


echo -n 'Preparing desktop file...'
cd ${BASEDIR}
rm -f whiskermenu.desktop.in
cp whiskermenu.desktop whiskermenu.desktop.in
sed -e '/Name\[/ d' \
	-e '/Comment\[/ d' \
	-e 's/Name/_Name/' \
	-e 's/Comment/_Comment/' \
	-i whiskermenu.desktop.in
cd ${WDIR}
echo ' DONE'


echo -n 'Updating translations...'
for POFILE in *.po;
do
	POLANG="${POFILE%%.*}"
	echo -n " $POLANG"
	msgunfmt "/usr/share/locale/$POLANG/LC_MESSAGES/gtk30.mo" > "gtk30-$POFILE"
	msgmerge --quiet --update --backup=none --compendium="gtk30-$POFILE" $POFILE xfce4-whiskermenu-plugin.pot
	rm -f "gtk30-$POFILE"
done
echo ' DONE'


echo -n 'Merging desktop file translations...'
cd ${BASEDIR}
intltool-merge --quiet --desktop-style ${WDIR} whiskermenu.desktop.in whiskermenu.desktop
rm -f whiskermenu.desktop.in
echo ' DONE'
