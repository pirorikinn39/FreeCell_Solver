#!/bin/bash -eux
bin/build.sh
( paste -d , <( for i in {1..100}; do bin/solver1 $i; done )     \
             <( for i in {1..100}; do bin/solver2 $i; done ) )   \
    | awk -F',' '{ print $1 "," $2 "," $3 "," $4 "," $8 "," $9 }' \
    | gzip -9 - - > data/ref.txt.gz
