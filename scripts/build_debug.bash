#!/bin/bash

g++ -g *.cpp -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/ddupes_debug.o
gdb build/ddupes_debug.o