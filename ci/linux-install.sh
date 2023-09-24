#!/usr/bin/env sh

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get -q update

sudo apt-get install -y lcov ruby

sudo apt-get install -y gcc-12 g++-12

# add dependencies for gtk3
sudo apt-get install -y liblzma5 libjbig0 libgtk-3-dev

# coreutils needed for uniq and sort used in testing
sudo apt-get install -y coreutils

# extra libs
sudo apt-get install -y libjpeg-dev
sudo apt-get install -y zlib1g-dev
sudo apt-get install -y libpng-dev
