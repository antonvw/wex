#!/usr/bin/env sh

brew update || brew update
brew install wxwidgets
brew unlink cmake
brew install cmake
