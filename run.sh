#!/bin/sh

cd build && cmake .. && make && ./netcat $1 $2 $3
