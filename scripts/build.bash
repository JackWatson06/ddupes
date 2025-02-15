#!/bin/bash

g++ *.cpp -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/ddupes.o
