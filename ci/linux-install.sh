#!/usr/bin/env sh

sudo apt-get install -y lcov ruby
gem install coveralls-lcov

# add dependencies for gtk2
sudo apt-get install -y liblzma5 libjbig0 libgtk2.0-0 libgtk2.0-common libgtk2.0-dev

# coreutils needed for uniq and sort used in testing
sudo apt-get install -y coreutils
