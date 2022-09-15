#!/bin/bash

for i in {4000..6000}
do
    ./gen.py $i > "test.json"
    ../build/Debug/json.exe test.json
    code=$?
    if [[ $code -ne 0 ]]; then
        mv test.json "$i.json"
    fi
done
