#!/bin/sh

xgettext --language=C++ --keyword=_ --package-name="whiskermenu" --package-version="1.0.0" `find src -name \*.cpp | sort` -o po/xfce4-whiskermenu-plugin.pot

for pofile in `find po -name \*.po`;
do
	msgmerge --quiet --update --backup=none $pofile po/xfce4-whiskermenu-plugin.pot
done
