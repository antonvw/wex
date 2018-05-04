#!/usr/bin/env sh

sudo apt-get install -y lcov ruby
gem install coveralls-lcov

# add dependencies for libwxgtk3
sudo apt-get install -y liblzma5 libjbig0

# coreutils needed for uniq and sort used in testing
sudo apt-get install -y coreutils
