#! /bin/bash

SERENDIP6_DIR=/data/Wei/FAST/serendip6
redis_file=redis_info.json
if [ -f $redis_file ]; then
rm redis_info.json
fi

cd /data/serendip6_reobs

$SERENDIP6_DIR/reobs_scripts/reobs_s6_stop.sh
sleep 5

$SERENDIP6_DIR/src/s6_redis/Redis_Demo 1>stdout.txt 2>stderr.txt &
sleep 1

#cd /data/serendip6_reobs
#$SERENDIP6_DIR/src/s6_reobs_init_fast.sh `mybeam.sh`
$SERENDIP6_DIR/src/s6_reobs_init_lab.sh
