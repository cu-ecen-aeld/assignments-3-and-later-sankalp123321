writefile=.
writestr=""
if [ $# -ne 2 ]
then
    echo "Invalid number of argumnets."
    echo "Argument 1: Full file path."
    echo "Argument 2: Text to be written into the file." 
    exit 1
else
    writefile=$1
    writestr=$2
fi

touch ${writefile}
if [ ! -f ${writefile} ]
then
    echo "File not created, re run the command."
    exit 1
fi

echo ${writestr} > ${writefile}


