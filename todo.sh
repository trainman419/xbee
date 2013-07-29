#!/bin/bash

TODO=`grep TODO *.h *.cpp | wc -l`
TODO=$((TODO))
echo "TODOs: $TODO"
