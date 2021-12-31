#!/bin/bash

instance=1
VERS6SW=0.0.1
VERS6GW=0.0.1
bindhost="test"
gpudev=0
beam=1
pol=0
netcpu=5
gpucpu=6
outcpu=7
net_thread="s6_fake_net_thread"
hashpipe -p serendip6 -I $instance   \
    -o VERS6SW=$VERS6SW                \
    -o VERS6GW=$VERS6GW                \
    -o RUNALWYS=1                      \
    -o MAXHITS=2048                    \
	-o POWTHRSH=40					   \
    -o BINDHOST=$bindhost              \
    -o BINDPORT=12345                  \
    -o GPUDEV=$gpudev                  \
    -o FASTBEAM=$beam                  \
    -o FASTPOL=$pol                    \
    -c $netcpu $net_thread             \
    -c $gpucpu s6_gpu_thread           \
    -c $outcpu s6_output_thread