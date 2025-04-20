#!/bin/bash

g++ tests/$1 -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test.o && build/test.o && echo "Success"
