#!/bin/sh

ARCHIVE=`grep "toktok/$1" MANIFEST | sed -e "s|^toktok/$1 ||"`
mkdir -p "$$"
tar zxf "$ARCHIVE" -C "$$"
if [ "$2" == "--debug" ]; then
  devenv -debugexe "$$/qtox/qtox.exe"
else
  "$$/qtox/qtox"
fi
rm -r $$
