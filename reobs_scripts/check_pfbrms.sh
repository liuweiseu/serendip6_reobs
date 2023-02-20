#! /bin/bash

for i in `host_list.sh`
do
echo checking pfb rms  on $i
ssh $i "hashpipe_check_status -I 1 -Q RMS_RE_AFT"
ssh $i "hashpipe_check_status -I 1 -Q RMS_IM_AFT"
ssh $i "hashpipe_check_status -I 2 -Q RMS_RE_AFT"
ssh $i "hashpipe_check_status -I 2 -Q RMS_IM_AFT"
done
