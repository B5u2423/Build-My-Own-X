#! /bin/bash

# hardcode the hash here 
cd ../bin 
echo -n 'Enter the tree hash: '
read hash
echo '----------------------------------'
./my-git ls-tree $hash
