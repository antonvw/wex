#!/usr/bin/env sh

lv="llvm@8"

export CC=/usr/local/opt/${lv}/bin/clang
export CXX=/usr/local/opt/${lv}/bin/clang
export LDFLAGS="-L/usr/local/opt/${lv}/lib -Wl,-rpath,/usr/local/opt/${lv}/lib"
export CPPFLAGS=-I/usr/local/opt/${lv}/include
