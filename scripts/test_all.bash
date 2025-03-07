#!/bin/bash

echo "Running all tests!"
for TEST_FILE in tests/*.cpp 
do  
  echo "Running $TEST_FILE";
  g++ $TEST_FILE src/*.cpp src/**/*.cpp -L/usr/lib -lssl -lcrypto -lsqlite3 -o build/test.o && build/test.o && echo "Success"
done
echo "Done!"