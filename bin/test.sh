#!/bin/bash -u
for i in {1..100}
do
    printf "Game #%04d: solver1..." $i
    cmp -s <( bin/solver1 $i | awk -F',' '{print $1 "," $2 "," $3}' ) \
           <( gzip -dc data/ref.txt.gz | awk -F',' "NR == $i {print \$1 \",\" \$2 \",\" \$3}" )
    if [ $? -gt 0 ]
    then echo "ERROR!"; exit 1
    else printf "PASSED, solver2..."
    fi
    cmp -s <( bin/solver2 $i | awk -F',' '{print $1 "," $2 "," $3}' ) \
           <( gzip -dc data/ref.txt.gz | awk -F',' "NR == $i {print \$1 \",\" \$2 \",\" \$3}" )
    if [ $? -gt 0 ]
    then echo "ERROR!"; exit 1
    else printf "PASSED\n"
    fi
done
