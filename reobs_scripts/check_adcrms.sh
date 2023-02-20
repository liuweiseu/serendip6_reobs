#! /bin/bash

for i in `host_list.sh`
do
echo checking adc rms  on $i
ssh $i "hashpipe_check_status -I 1 -Q ADCRMS"
ssh $i "hashpipe_check_status -I 2 -Q ADCRMS"
done
