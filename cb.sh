#!/bin/bash

cb_root=$(dirname -- "$0")
cb_gcc=1
cb_compiler="${CC:-gcc}"
cb_run=1
cb_file="cb.c"       # Source file name to compile
cb_output="./cb.bin"   # Executable name
cb_include_dir="$cb_root/"    # Include directory to locate the cb.h file

while (( "$#" )); do

    echo "$1"
    if [ "$1" == "clang" ];  then clang=1; cb_compiler="${CC:-clang}"; unset cb_gcc; fi
    if [ "$1" == "gcc" ];    then cb_gcc=1;   cb_compiler="${CC:-gcc}";   unset clang; fi
    if [ "$1" == "help" ];   then cb_help=1; fi
    if [ "$1" == "run" ];    then cb_run=1; fi    
    if [ "$1" == "--file" ]; then cb_file=$2; shift; fi
    if [ "$1" == "--output" ]; then cb_output=$2; shift; fi 
	if [ "$1" == "--include-dir" ]; then cb_include_dir=$2; shift; fi 
    shift

done

# Remove previous executable if it exists.
if [ -f $cb_output ]; then
   rm $cb_output
fi

# Check if there is a value in cb_gcc.
if [ -v cb_gcc ]; then
   $cb_compiler -std=c89 -g -I $cb_include_dir -o $cb_output -O0 $cb_file || { echo "'$cb_compiler' exited with $?"; exit 1; }
fi

# Check if there is a value in cb_run.
if [ -v cb_run ]; then
   $cb_output || { echo "'$cb_output' exited with $?"; exit 1; }
fi