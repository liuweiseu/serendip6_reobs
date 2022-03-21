#!/bin/bash

instance=0
VERS6SW=0.0.2
VERS6GW=0.0.2
#iface_pol0=`myinterface.sh voltpol0`
#iface_pol1=`myinterface.sh voltpol1`

workdir=$(cd $(dirname $0); pwd)
bindhost=${iface_pol0}
freq_range="1.05G-1.45G"
gpudev=0
beam=1
pol=0
netcpu=5
gpucpu=6
outcpu=7
gain=1.0
wfile=$workdir"/fir_weights/matlab_fir_weights.dat"
net_thread="s6_fake_net_thread"
compute_node=$(hostname)
echo $net_thread

hashpipe -p ./serendip6_reobs.so -I $instance   \
    -o VERS6SW=$VERS6SW                \
    -o VERS6GW=$VERS6GW                \
    -o RUNALWYS=1                      \
    -o BINDHOST=$bindhost              \
    -o BINDPORT=12345                  \
    -o GPUDEV=$gpudev                  \
    -o FASTBEAM=$beam                  \
    -o FASTPOL=$pol                    \
    -o WEIGHTS=$wfile                  \
    -o NEWFILE=0                       \
    -o GAIN=$gain                      \
    -o COMPUTE_NODE=$compute_node      \
    -o FREQ=$freq_range                \
    -c $netcpu $net_thread             \
    -c $gpucpu s6_gpu_thread           \
    -c $outcpu s6_output_thread