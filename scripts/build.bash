#!/bin/bash

g++ -g *.cpp -L/usr/lib -lssl -lcrypto -o build/ddupes.o
