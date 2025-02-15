#!/bin/bash

g++ -g tests/test_$1.cpp $(ls *.cpp | grep -v "ddupes.cpp") -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test_debug.o
gdb "build/test_debug.o"