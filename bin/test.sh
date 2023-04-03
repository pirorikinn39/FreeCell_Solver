#!/bin/bash -ue
OPT=("")
FULL=no
TARGETS=(solver1 solver2)
for arg in $@
do
    case $arg in
	solver1 ) TARGETS=(solver1); OPT+=($arg);;
	solver2 ) TARGETS=(solver2); OPT+=($arg);;
	full )    FULL=yes;;
	* )       OPT+=($arg);;
    esac
done

if [ $FULL == yes ]
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

bin/build.sh ${OPT[@]}
for i in {1..100}
do
    if [[ "${TARGETS[*]}" =~ solver1 ]]; then
	printf "Game #%04d: solver1..." $i
	if ! cmp -s <( bin/solver1 $i | awk -F',' "$PROGRAM_1a" ) \
             <( gzip -dc data/ref.txt.gz | awk -F',' "NR == $i $PROGRAM_1b" )
	then echo ERROR!; break
	else echo PASSED
	fi
    fi
    
    if [[ "${TARGETS[*]}" =~ solver2 ]]; then
	printf "Game #%04d: solver2..." $i
	if ! cmp -s <( bin/solver2 $i | awk -F',' "$PROGRAM_2a" ) \
             <( gzip -dc data/ref.txt.gz | awk -F',' "NR == $i $PROGRAM_2b" )
	then echo ERROR!; break
	else echo PASSED
	fi
    fi
done
