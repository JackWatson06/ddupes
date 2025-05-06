#!/bin/bash

g++ -g tests/$1 -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test_debug.o 
gdb "build/test_debug.o"