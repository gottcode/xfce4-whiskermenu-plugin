#!/bin/sh


BASEDIR=../panel-plugin


echo -n 'Extracting messages...'
# Sort alphabetically to match cmake
xgettext --from-code=UTF-8 --c++ --keyword=_ --sort-output \
	--package-name='Whisker Menu' --copyright-holder='Graeme Gott' \
	--output=xfce4-whiskermenu-plugin.pot ${BASEDIR}/*.cpp ${BASEDIR}/*.h
xgettext --from-code=UTF-8 -k --keyword=Name --keyword=Comment --join-existing --sort-output \
	--package-name='Whisker Menu' --copyright-holder='Graeme Gott' \
	--output=xfce4-whiskermenu-plugin.pot ${BASEDIR}/*.desktop.in
echo ' DONE'
