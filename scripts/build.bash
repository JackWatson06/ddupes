#!/bin/bash

g++ extract.cpp extract_output.cpp load.cpp transform.cpp ddupes.cpp -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/ddupes.o
