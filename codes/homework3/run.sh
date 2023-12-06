#!/bin/bash
if [ $# -lt 3 ];
then
    echo "wrong arguments"
else
    ./matgen $1 $2 input1.txt
    ./print input1.txt
    ./matgen $2 $3 input2.txt
    ./print input2.txt
    ./serial input1.txt input2.txt serial_out.txt
    ./print serial_out.txt
fi