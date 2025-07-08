#!/bin/bash
#REM Run from project root
mkdir -p build
cd build
cmake -S ../ -b .
make && make Shaders && ./SJFEngine
cd ..