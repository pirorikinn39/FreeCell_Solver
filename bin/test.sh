#!/bin/bash -ue
OPT=("")
FULL=no
for arg in $@
do
    case $arg in
	full ) FULL=yes;;
	* )    OPT+=($arg);;
    esac
done

if [ $FULL == yes ];
then
    PROGRAM_1a='{print $1 "," $2 "," $3 "," $4}'
    PROGRAM_1b='{print $1 "," $2 "," $3 "," $4}'
    PROGRAM_2a='{print $1 "," $2 "," $3 "," $4 "," $5}'
    PROGRAM_2b='{print $1 "," $2 "," $3 "," $5 "," $6}'
else
    PROGRAM_1a='{print $1 "," $2}'
    PROGRAM_1b='{print $1 "," $2}'
    PROGRAM_2a='{print $1 "," $2}'
    PROGRAM_2b='{print $1 "," $2}'
fi

bin/build.sh $OPT
for i in {1..100}
do
    printf "Game #%04d: solver1..." $i
    cmp -s <( bin/solver1 $i | awk -F',' "$PROGRAM_1a" ) \
           <( gzip -dc data/ref.txt.gz | awk -F',' "NR == $i $PROGRAM_1b" )
    if [ $? -gt 0 ]
    then echo "ERROR!"; break
    else printf "PASSED, solver2..."
    fi
    cmp -s <( bin/solver2 $i | awk -F',' "$PROGRAM_2a" ) \
           <( gzip -dc data/ref.txt.gz | awk -F',' "NR == $i $PROGRAM_2b" )
    if [ $? -gt 0 ]
    then echo "ERROR!"; break
    else printf "PASSED\n"
    fi
done
