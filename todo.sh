#!/bin/bash

TODO=`grep TODO *.h *.cpp *.c | wc -l`
FAKE=`grep fake_cmd *.h *.cpp *.c | wc -l`
echo "TODOs: $TODO"
echo "Fake Commands: $FAKE"
