#!/bin/bash

g++ ddupes.cpp src/*.cpp src/**/*.cpp -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/ddupes.o
