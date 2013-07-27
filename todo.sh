#!/bin/bash

TODO=`grep TODO *.h *.cpp | wc -l`
FAKE=`grep fake_cmd *.h *.cpp | wc -l`
FAKE=$(( $FAKE - 2 ))
echo "TODOs: $TODO"
echo "Fake Commands: $FAKE"
