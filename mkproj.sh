#!/bin/bash
set +x
mkdir -p project/ && cd project
cmake -DCMAKE_BUILD_TYPE=Debug -G"Eclipse CDT4 - Unix Makefiles" ../src
