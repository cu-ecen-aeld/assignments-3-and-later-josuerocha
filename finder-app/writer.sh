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

mkdir -p $(dirname "$path")
echo "$content" > $path