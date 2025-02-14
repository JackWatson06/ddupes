#!/bin/bash

case "$1" in
  "extract")
    g++ -g test_extract.cpp extract.cpp -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test_extract.o
    gdb "build/test_extract.o"
    ;;
  "transform")
    g++ -g test_transform.cpp transform.cpp -L/usr/lib -lssl -lcrypto -o build/test_transform.o
    gdb "build/test_transform.o"
    ;;
  "load")
    g++ -g test_load.cpp load.cpp -L/usr/lib -lssl -lcrypto -o build/test_load.o
    gdb "build/test_load.o"
    ;;
  *)
    echo "Please pass in the file you want to debug. Options are 'transform', or 'load'".
    ;;
esac
