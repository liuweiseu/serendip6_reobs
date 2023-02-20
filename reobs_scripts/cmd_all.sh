#!/bin/sh

for i in `host_list.sh` 
do 
echo execute $1 on $i
ssh $i $1
done
