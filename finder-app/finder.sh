#!/bin/sh

dirpath=$1
searchstr=$2

echo Dirpath: $1
echo Search Str: $2
echo Num param: $#

if [ "$#" -ne 2 ];
    then
        echo "Didn't supply all parameters"
	return 1
fi

if [ ! -d "$1" ];
    then 
	echo "Not a valid folder"
	return 1
fi

numfiles=$(find "$1" -type f | wc -l)
matches=$(grep -r "$2" "$1" | wc -l)

echo "The number of files are" $numfiles "and the number of matching lines are "$matches

return 0

