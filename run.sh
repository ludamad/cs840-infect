#!/bin/bash
set +x
mkdir -p build/ 
cd build 
cmake -DCMAKE_BUILD_TYPE=Debug ../src
make -j5 && cd ../src && gdb -silent -ex=r -ex=q ../build/infectsim
