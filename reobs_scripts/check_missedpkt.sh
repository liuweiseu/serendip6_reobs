#! /bin/bash

for i in `host_list.sh`
do
echo checking missed pkts  on $i
ssh $i "hashpipe_check_status -I 1 -Q MISSEDPK"
ssh $i "hashpipe_check_status -I 2 -Q MISSEDPK"
done
