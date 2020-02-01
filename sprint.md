# todo

# backlog
- use clang-9 (llvm@9, first on MacBook) (after macOS 10.15 deployed on travis)
- use gcc-9 (check on linux)
  - enable lcov
- wxDateTime replace by c++20 methods date and time utilities

# doc

# Get and install boost
wget https://dl.bintray.com/boostorg/release/<>/source/boost_<>.tar.gz
tar -xzf boost_<>*
cd boost_<>*
./bootstrap.sh --prefix=/opt/boost
./b2
sudo ./b2 install --prefix=/opt/boost --with=all

## select gcc 8 and cmake
yum install centos-release-scl
yum install devtoolset-8
cmake3 -DBOOST_ROOT=/opt/boost -DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gcc -DCMAKE_C_COMPILER=/opt/rh/devtoolset-8/root/usr/bin/gcc -DCMAKE_BUILD_TYPE=Debug ..

## in shell
scl enable devtoolset-8 bash
export CC=gcc && export CXX=gcc
make

## in common.cmake patch boost
add (above m)
/opt/boost/lib/libboost_program_options.a
