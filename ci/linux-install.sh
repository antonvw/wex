#!/usr/bin/env sh

sudo apt-get install -y lcov ruby
gem install coveralls-lcov

# add dependencies for gtk2
sudo apt-get install -y liblzma5 libjbig0 libgtk2.0-0 libgtk2.0-common libgtk2.0-dev

# coreutils needed for uniq and sort used in testing
sudo apt-get install -y coreutils

# get and unpack boost
wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz
tar -xzf boost_1_*
