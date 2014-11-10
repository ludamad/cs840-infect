#!/bin/bash
set -e
function process () {
   convert $1 $2
   rm $1
}
echo HELP ME FOR $1 $2
process $1 $2 &
