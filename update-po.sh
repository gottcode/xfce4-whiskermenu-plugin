#!/bin/sh

xgettext --language=C++ --keyword=_ --package-name="Whisker Menu" `find src -name \*.cpp | sort` -o po/xfce4-whiskermenu-plugin.pot

for pofile in `find po -name \*.po`;
do
	msgmerge --quiet --update --backup=none $pofile po/xfce4-whiskermenu-plugin.pot
done
