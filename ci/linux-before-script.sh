#!/usr/bin/env sh

sh -e /etc/init.d/xvfb start

sudo rm /usr/bin/g++
sudo ln -s /usr/bin/g++-6.0 /usr/bin/g++
sudo rm /usr/bin/gcov
sudo ln -s /usr/bin/gcov-6.0 /usr/bin/gcov
