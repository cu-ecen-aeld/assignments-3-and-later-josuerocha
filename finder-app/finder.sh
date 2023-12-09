#!/usr/bin/env bash

if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    exit 1
fi

path=$1
content=$2

if [ -z ${path+x} ]; then 
echo "0 is unset"
exit 1
fi

if [ -z ${content+x} ]; then 
echo "1 is unset" 
exit 1
fi

echo Path: $path Content: $content

if [ -d $path ]
then
echo "The directory exists"
else 
echo "The directory does not exist"
exit 1
fi

number_files=$(find "${path}" -type f | wc -l)

number_lines=$(grep -ro "${content}" "${path}" | wc -l)

echo The number of files are $number_files and the number of matching lines are $number_lines