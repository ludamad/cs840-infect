#!/bin/bash
set +x
mkdir -p build/ 
cd build 

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


if handle_flag "-O" ; then
    cmake -DCMAKE_BUILD_TYPE=Release ../src
else
    cmake -DCMAKE_BUILD_TYPE=Debug ../src
fi
make -j5 && cd ../src && gdb -silent -ex=r -ex=q ../build/infectsim
