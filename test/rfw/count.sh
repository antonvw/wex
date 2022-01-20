#!/bin/bash

files=$(find ./*robot* | wc -l | xargs)
echo "$files"
