#! /bin/bash

echo 'Building project'
cd .. && make

echo '----------------------------------'
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
# echo 'file1' > file1.txt
# echo 'file2' > file2.txt

# # Sub-tree ~ directory

# mkdir "dir1" > /dev/null 2>&1
# mkdir "dir2" > /dev/null 2>&1
# echo 'dummy1' > dir1/dumfile2.txt # Occupy the directory to be included in the tree
# echo 'dummy2' > dir2/dumfile1.txt
# echo 'Added sub directories and files'

# git add .
# echo "New git tree written"
# git write-tree
# echo '----------------------------------'
