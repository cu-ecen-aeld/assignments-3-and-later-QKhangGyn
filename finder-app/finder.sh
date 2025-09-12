#!/bin/sh 
NUM_ARG=2 
filedir=$1 
searchstr=$2
# Condition check
#
# Number of argument check 
if [ $# -ne ${NUM_ARG} ]
then
	echo "ERROR: Invalid number of arguments.
Total number of arguments should be 1.
The order of the arguments should be:
1. File directory path
2. String to be searched in the specified directory path"
	exit 1
#Is the 1st arg is the directory in the file system
elif ! [ -d $filedir ]
then	
	echo "Hey the filesystem directory path is not directory"
	exit 1
else
	X=$( find $filedir -type f | wc -l )
	Y=$( grep -r "$searchstr" "$filedir" | wc -l )
	echo "The number of files are ${X} and the number of matching lines are ${Y}"
	exit 0
fi
