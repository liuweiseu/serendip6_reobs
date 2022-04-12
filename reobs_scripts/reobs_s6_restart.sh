#! /bin/bash


./reobs_s6_stop.sh 
sleep 5 
cd /data/serendip6_reobs
/home/obs/wei/serendip6/src/s6_reobs_init_fast.sh `mybeam.sh`

