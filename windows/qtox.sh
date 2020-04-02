#!/bin/sh

ARCHIVE=`grep "toktok/$1" MANIFEST | sed -e "s|^toktok/$1 ||"`
mkdir -p "$$"
tar zxf "$ARCHIVE" -C "$$"
"$$/qtox/qtox"
rm -r $$
