#!/bin/bash

for i in {1024..2048}
do
    echo "-------------------------------------- $i"
    ./gen.py $i > "fuzz.json"
    ../build/Debug/json.exe "fuzz.json"
#    code=$?
#    if [[ $code -ne 0 ]]; then
#        mkdir fails
#        mv "$i.json" "fails/fuzz$i.json"
#    fi
done
