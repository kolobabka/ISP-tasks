#!/bin/bash 
for file in ./build/*.dot
do
    fileName=${file#\.*d}
    fileName=${fileName%*.dot}.png
    dot -Tpng ${file} -o pics${fileName}
done