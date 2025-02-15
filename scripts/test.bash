#!/bin/bash

g++ tests/test_$1.cpp $(ls *.cpp | grep -v "ddupes.cpp") -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test.o && build/test.o && echo "Success"