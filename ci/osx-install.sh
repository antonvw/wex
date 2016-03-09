#!/usr/bin/env sh

brew update || brew update
brew install gtk+3
brew install wxwidgets
brew unlink cmake
brew install cmake30
