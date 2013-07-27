#!/bin/bash

TODO=`grep TODO *.h *.cpp | wc -l`
TODO=$((TODO))
FAKE=`grep fake_cmd *.h *.cpp | wc -l`
CHILDREN=`grep 'new command_child(' *.h *.cpp | wc -l`
CHILDREN=$((CHILDREN))
FAKE=$(( FAKE - 2 ))

PERCENT=$(( ( CHILDREN - FAKE ) * 100 ))
PERCENT=$(( PERCENT / CHILDREN ))
echo "TODOs:            $TODO"
echo "Fake Commands:    $FAKE of $CHILDREN"
echo "Percent complete: $PERCENT%"
