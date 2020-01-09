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


echo -n 'Updating translations... '
for POFILE in *.po;
do
	POLANG="${POFILE%%.*}"
	echo -n "$POLANG "

	if [ -f "/usr/share/locale/$POLANG/LC_MESSAGES/xfce4-appfinder.mo" ]; then
		msgunfmt "/usr/share/locale/$POLANG/LC_MESSAGES/xfce4-appfinder.mo" > "appfinder-$POFILE"
	else
		touch "appfinder-$POFILE"
	fi

	if [ -f "/usr/share/locale/$POLANG/LC_MESSAGES/xfce4-panel.mo" ]; then
		msgunfmt "/usr/share/locale/$POLANG/LC_MESSAGES/xfce4-panel.mo" > "panel-$POFILE"
	else
		touch "panel-$POFILE"
	fi

	if [ -f "/usr/share/locale/$POLANG/LC_MESSAGES/gtk30.mo" ]; then
		msgunfmt "/usr/share/locale/$POLANG/LC_MESSAGES/gtk30.mo" > "gtk30-$POFILE"
	else
		touch "gtk30-$POFILE"
	fi

	msgmerge --quiet --update --backup=none \
		--compendium="appfinder-$POFILE" \
		--compendium="panel-$POFILE" \
		--compendium="gtk30-$POFILE" \
		$POFILE xfce4-whiskermenu-plugin.pot

	rm -f "appfinder-$POFILE" "panel-$POFILE" "gtk30-$POFILE"
done
echo 'DONE'


echo -n 'Merging desktop file translations...'
cd ${BASEDIR}
intltool-merge --quiet --desktop-style ${WDIR} whiskermenu.desktop.in whiskermenu.desktop
rm -f whiskermenu.desktop.in
echo ' DONE'
