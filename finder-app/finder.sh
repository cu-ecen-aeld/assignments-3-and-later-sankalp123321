#!/bin/sh
# Finder script

# Setting the default directory location to pwd
filesdir=./
searchstr=""
filecount=0
strcount=0

# If the argument count is no 2, error.
if [ $# -ne 2 ] 
then
    echo "Invalid number of argumnets."
    echo "Argument 1: Directory to be searched."
    echo "Argument 2: Text to be searched." 
    exit 1
else
# Assign the arguments to the variables.
    filesdir=$1
    searchstr=$2
fi

# Check if the path provided is a directory or not.
if [ ! -d ${filesdir} ]
then
    echo "The path provided is not a directory."
    exit 1
fi

# Get the total file count
filecount=$(find ${filesdir} | wc -l)
# Get the string match count
strcount=$(grep -r ${searchstr} ${filesdir} | wc -l)
# Reduce the count by 1 because current dir '.' is also counted.
filecount=$((filecount-1))

# Print Log
echo "The number of files are ${filecount} and the number of matching lines are ${strcount}"
