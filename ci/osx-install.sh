#!/usr/bin/env sh

brew update || brew update
brew install gtk+3
brew install wxwidgets
brew unlink gcc
brew install gcc49
brew link --force gcc49
