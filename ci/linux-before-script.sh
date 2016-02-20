#!/usr/bin/env sh

# we need  a display for the gui tests
export DISPLAY=:99.0

# take care that recently-used.xbel file can be written
export XDG_DATA_HOME=$PWD

sh -e /etc/init.d/xvfb start

sudo rm /usr/bin/g++
sudo ln -s /usr/bin/g++-4.9 /usr/bin/g++
sudo rm /usr/bin/gcov
sudo ln -s /usr/bin/gcov-4.9 /usr/bin/gcov
