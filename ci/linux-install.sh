#!/usr/bin/env sh

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install -y g++-6.0
sudo apt-get install -y lcov ruby
gem install coveralls-lcov

# first add dependencies for libwxgtk3
sudo apt-get install -y liblzma5 libjbig0
sudo apt-get install -y libwxbase3.0-0 libwxgtk3.0-0 libwxbase3.0-dev libwxgtk3.0-dev wx3.0-headers wx-common wx3.0-i18n    

# coreutils needed for uniq and sort used in testing
sudo apt-get install -y coreutils
