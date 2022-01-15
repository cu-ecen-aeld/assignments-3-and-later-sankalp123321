# Setting the default directory location to pwd
writefile=.
writestr=""

# If the argument count is no 2, error.
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
# Create the file in the directory.
touch ${writefile}
# Check of the file/directory was created successfullly.
if [ ! -f ${writefile} ]
then
    echo "File not created, re run the command."
    exit 1
fi
# Populate the file.
echo ${writestr} > ${writefile}


