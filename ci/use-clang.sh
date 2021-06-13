#!/usr/bin/env sh

lv="llvm@12"

export CC=/usr/local/opt/${lv}/bin/clang
export CXX=/usr/local/opt/${lv}/bin/clang

export LDFLAGS="-L/usr/local/opt/${lv}/lib -Wl"
export CPPFLAGS=-I/usr/local/opt/${lv}/include
