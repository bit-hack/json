#!/bin/bash

rm -rf fuzz*.c

for i in {2000..4000}
do
    python gen.py $i > "test.json"
    ../build/Debug/json.exe test.json
    code=$?
    if [[ $code -ne 0 ]]; then
        mv test.json "$i.json"
    fi
done
