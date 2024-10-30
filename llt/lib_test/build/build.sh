#!/bin/bash
mkdir -p ../output
cd ../output
cmake ../src -G "Unix Makefiles" -D BUILD_GMOCK=OFF
make