#!/usr/bin/env sh

sudo apt-get install -y lcov ruby
gem install coveralls-lcov

# add dependencies for gtk3
sudo apt-get install -y liblzma5 libjbig0 libgtk-3-dev

# coreutils needed for uniq and sort used in testing
sudo apt-get install -y coreutils
