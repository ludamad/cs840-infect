#!/bin/bash
set +x

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
        unbuffer $prog $args | util/colorify.sh
    else
        $prog $args
    fi
}

if handle_flag "--debug" || handle_flag "--gdb" || handle_flag "-g" ; then
    RELEASETYPE='debug'
    mkdir -p build/debug 
    cd build/debug
    cmake -DCMAKE_BUILD_TYPE=Debug ../../src
else
    RELEASETYPE='release'
    mkdir -p build/release
    cd build/release
    cmake -DCMAKE_BUILD_TYPE=Release ../../src
fi

make -j5 && runit
