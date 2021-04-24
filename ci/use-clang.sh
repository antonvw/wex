#!/usr/bin/env sh

lv="llvm@9"

export CC=/usr/local/opt/${lv}/bin/clang
export CXX=/usr/local/opt/${lv}/bin/clang

export LDFLAGS="-L/usr/local/opt/${lv}/lib -L/usr/local/opt/icu4c/lib -Wl"
export CPPFLAGS=-I/usr/local/opt/${lv}/include
