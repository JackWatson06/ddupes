#!/bin/bash

CPP_FILES=$(ls *.cpp | grep -v "ddupes.cpp")

echo "Running all tests!"
for TEST_FILE in tests/*.cpp 
do  
  echo "Running $TEST_FILE";
  g++ $TEST_FILE $CPP_FILES -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test.o && build/test.o && echo "Success"
done
echo "Done!"