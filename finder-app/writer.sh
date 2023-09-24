#!/bin/sh

filepath=$1
strtowrite=$2

echo filepath: $1
echo string: $2
echo Num param: $#

if [ "$#" -ne 2 ];
    then
        echo "Didn't supply all parameters"
	return 1
fi

dirname=$(dirname "$1")
echo $dirname

if [ ! -d "$dirname" ]
    then
        mkdir -p "$dirname"
fi

echo "$2" > "$1"

if [ "$?" -ne 0 ];
    then
        echo "Failed to write"
	return 1
fi

return 0
