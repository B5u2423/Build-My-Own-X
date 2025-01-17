#! /bin/bash

make

cd bin && ./my-git init

echo '----------------------------------'
echo 'Hash test string to test cat-file'
echo 'this is a test string' | git hash-object -w --stdin
echo 'Done.'

echo '----------------------------------'
file_name="newfile.txt"
echo 'Make new file for hash-object'
echo 'this is a test file' > "$file_name" 
echo "Content written to $file_name"
