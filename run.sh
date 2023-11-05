#!/bin/bash

# Check if build directory exists, if not, create it
if [ ! -d "build" ]; then
  mkdir build
fi

# Navigate to the build directory
cd build

# Run cmake and make
cmake ..
make

# Execute the built program
./updog $1 $2 $3
