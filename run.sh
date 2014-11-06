#!/bin/bash
set +x
mkdir -p build/ && cd build && cmake ../src && make -j5
./infectsim
