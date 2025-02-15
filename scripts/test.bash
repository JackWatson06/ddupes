#!/bin/bash

case "$1" in
  "extract")
    echo "Running extract tests"
    g++ test_extract.cpp extract.cpp extract_output.cpp -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test_extract.o && build/test_extract.o && echo "Success"
    ;;
  "transform")
    echo "Running transform tests"
    g++ test_transform.cpp transform.cpp extract_output.cpp -L/usr/lib -lssl -lcrypto -o build/test_transform.o && build/test_transform.o && echo "Success"
    ;;
  "load")
    echo "Running load tests"
    g++ test_load.cpp load.cpp -L/usr/lib -lssl -lcrypto -o build/test_load.o && build/test_load.o && echo "Success"
    ;;
  *)
    echo "Running all tests"
    g++ test_extract.cpp extract.cpp extract_output.cpp -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test_extract.o && build/test_extract.o && echo "Success"
    g++ test_transform.cpp transform.cpp  extract_output.cpp -L/usr/lib -lssl -lcrypto -o build/test_transform.o && build/test_transform.o && echo "Success"
    g++ test_load.cpp load.cpp -L/usr/lib -lssl -lcrypto -o build/test_load.o && build/test_load.o && echo "Success"
    ;;
esac
