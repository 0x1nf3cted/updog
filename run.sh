#!/bin/sh

cd build && cmake .. && make && ./updog $1 $2 $3
