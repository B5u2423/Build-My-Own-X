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

echo '----------------------------------'
echo "Writing tree"

echo 'file1' > file1.txt
echo 'file2' > file2.txt

# Sub-tree ~ directory
mkdir dir1
mkdir dir2
echo 'dummy1' > dir1/dumfile2.txt # Occupy the directory to be included in the tree
echo 'dummy2' > dir2/dumfile1.txt

git add .
git write-tree
