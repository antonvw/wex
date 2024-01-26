#!/usr/bin/env sh

lv="llvm@17"

export CC=/usr/local/Cellar/homebrew/opt/${lv}/bin/clang
export CXX=/usr/local/Cellar/homebrew/opt/${lv}/bin/clang

export LDFLAGS="-L/usr/local/Cellar/homebrew/opt/${lv}/lib"
export CPPFLAGS="-I/usr/local/Cellar/homebrew/opt/${lv}/include"
