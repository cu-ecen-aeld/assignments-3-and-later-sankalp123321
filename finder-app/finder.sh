#!/bin/sh
# Finder script

filesdir=./
searchstr=""
filecount=0
strcount=0
if [ $# -ne 2 ] 
then
    echo "Invalid number of argumnets."
    echo "Argument 1: Directory to be searched."
    echo "Argument 2: Text to be searched." 
    exit 1
else
    filesdir=$1
    searchstr=$2
fi

if [ -d filesdir ]
then
    echo "The path provided is not a directory."
    exit 1
fi

# echo "Directory ${filesdir} text ${searchstr}"
filecount=$(find ${filesdir} | wc -l)
strcount=$(grep -r ${searchstr} ${filesdir} | wc -l)
let filecount--

echo "The number of files are ${filecount} and the number of matching lines are ${strcount}"
