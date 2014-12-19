#!/bin/bash
set +x

function unbuffer() {
    ./util/unbuffer.sh "$@"
}

ROOTDIR="$(pwd)"

args="$@" # Create a mutable copy of the program arguments
function handle_flag(){
    flag=$1
    local new_args
    local got
    got=1 # False!
    for arg in $args ; do
        if [ $arg = $flag ] ; then
            args="${args/$flag/}"
            got=0 # True!
        else
            new_args="$new_args $arg"
        fi
    done
    args="$new_args"
    return $got # False!
}

function runit() {
    cd "$ROOTDIR"
    prog=build/$RELEASETYPE/infectsim
    if [ $RELEASETYPE = debug ] ; then
        gdb -silent -ex=r -ex=q --args $prog $args 
    elif true || handle_flag "-C" || handle_flag "--color" ; then
        unbuffer $prog $args #| util/colorify.sh
    else
        $prog $args
    fi
}

RECORD=""
if handle_flag "--record" ; then
    RECORD=1 
fi


if handle_flag "--debug" || handle_flag "--gdb" || handle_flag "-g" ; then
    RELEASETYPE='debug'
    mkdir -p build/debug 
    cd build/debug
    #cmake -DCMAKE_BUILD_TYPE=Debug -DEXTRA_DEFS="-fsanitize=address -D_GLIBCXX_DEBUG" ../../src
    cmake -DCMAKE_BUILD_TYPE=Debug -DEXTRA_DEFS="-D_GLIBCXX_DEBUG" ../../src
elif handle_flag "-st" ; then
    RELEASETYPE='st'
    mkdir -p build/st
    cd build/st
    cmake -DCMAKE_BUILD_TYPE=Release -DEXTRA_DEFS="-DSEARCH_STRUCT=DiscreteSearchTree" ../../src
elif handle_flag "-bst" ; then
    RELEASETYPE='bst'
    mkdir -p build/bst
    cd build/bst
    cmake -DCMAKE_BUILD_TYPE=Release -DEXTRA_DEFS="-DSEARCH_STRUCT=DiscreteBST" ../../src
elif handle_flag "-bt" ; then
    RELEASETYPE='bt'
    mkdir -p build/bt
    cd build/bt
    cmake -DCMAKE_BUILD_TYPE=Release -DEXTRA_DEFS="-DSEARCH_STRUCT=DiscreteBucketTree" ../../src
else 
    RELEASETYPE='release'
    mkdir -p build/release
    cd build/release
    cmake -DCMAKE_BUILD_TYPE=Release ../../src
fi

if [ $RECORD ] ; then
    make -j5 && runit &

    cd "$ROOTDIR"
    while true ; do
        wid=`wmctrl -lp | grep "Infection Simulation" | sed 's/^0x\(\w\+\)\s\+\(.*\)$/0x\1/'`
        if [ $wid ] ; then
            echo "Recording window ID $wid"
            recordmydesktop --windowid "$wid" -o rec.ogv --v_bitrate 200000000 --v_quality 0 --fps 60
            break
        fi
    done
else
    make -j5 && runit 
fi

