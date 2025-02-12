#!/bin/bash

g++ -g extract.cpp load.cpp transform.cpp ddupes.cpp -L/usr/lib -lssl -lcrypto -o build/ddupes.o
