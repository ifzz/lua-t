#!/bin/bash

if [ -d out ]; then
	rm -rf out
fi
mkdir out

cp src/* ./out/
cp ../include/qtc_types.h ./out/
cp library/library.h library/l_sock* library/l_byteBuffer* ./out/


# 
cp -f library/{Makefile,linit.c,lualib.h} ./out/
cd out
# this accumulates the Lua provided upstream patches
patch < ../patches.patch
make linux

cd ..
