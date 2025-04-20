#!/bin/bash

echo "Running all tests!"
for TEST_FILE in tests/**/test_*.cpp tests/test_*.cpp
do  
  echo "Running $TEST_FILE";
  g++ $TEST_FILE -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test.o && build/test.o && echo "Success"
done
echo "Done!"