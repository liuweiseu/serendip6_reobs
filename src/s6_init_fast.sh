#!/bin/bash

VERS6SW=0.8.0                   \
VERS6GW=0.1.0                   \

# Add directory containing this script to PATH
PATH="$(dirname $0):${PATH}"

hostname=`hostname -s`
#net_thread=${1:-s6_pktsock_thread}
net_thread="s6_pktsock_thread"
beam=$1
beam=$1

# Remove old semaphore
echo removing old semaphore, if any
rm /dev/shm/sem.serendip6_gpu_sem_device_*

# Setup parameters for two instances.
instance_i=("1" "2")
#instance_i=("1")
log_timestamp=`date +%Y%m%d_%H%M%S`
instances=(
  # NOTE: when changing any of the following it is good practice to run:
  # sudo ipcrm -a
  # in order to have initial shared memory allocations occur on the local NUMA node.
  #
  # Production config:
  # run s6 on NUMA node 1 (odd CPUs on FAST compute nodes) one time setup:
  # sudo ~jeffc/bin/set_irq_cpu.csh 292 00000200			# p2p3 interrupts go to CPU 9 
  # sudo ~jeffc/bin/set_irq_cpu.csh 326 00020000			# p2p4 interrupts go to CPU 17
  # sudo ~jeffc/bin/set_irq_cpu.csh 224 00000100			# NIC p2p1 interrupts to CPU 8 for FRB hashpipe
  # sudo mount -o remount mpol=bind:0 /mnt/fast_frb_data 	# mount FRB ramdisk on NUMA node 0   
  #
  # FRB hashpipe to use: 	numactl --physcpubind=18,20,22 --membind=0
  # heimdall to use: 		CPU 16 and GPU 0
  # and, optionally,
  # second heimdall to use:	CPU  6 and GPU 0
  #
  # hashpipe command line parameters (serendip6 will run as hashpipe instances 1 and 2):
  " place holder for unused instance 0.  FRB hashpipe uses instance 0"
  "--physcpubind=11,13,15 --membind=0,1 p2p3 1   11  13  15  ${beam} 0  $log_timestamp" # Instance 0
  "--physcpubind=19,21,23 --membind=0,1 p2p4 1   19  21  23  ${beam} 1  $log_timestamp" # Instance 1
);

function init() {
  instance=${1}
  numaops=${2}
  membind=${3}
  bindhost=${4}
  gpudev=${5}
  netcpu=${6}
  gpucpu=${7}
  outcpu=${8}
  beam=${9}
  pol=${10}
  log_timestamp=${11}

  if [ -z "${numaops}" ]
  then
    echo "Invalid instance number '${instance}' (ignored)"
    return 1
  fi

  if [ -z "$outcpu" ]
  then
    echo "Invalid configuration for host ${hostname} instance ${instance} (ignored)"
    return 1
  fi

  if [ $net_thread == 's6_pktsock_thread' ]
  then
    # AO
    #bindhost="eth$((2+2*instance))"
    # GBT
    # FAST
    #bindhost="p2p$((3+instance))"
    bindhost="p2p$((2+instance))"
    #bindhost="p2p1"
    #bindhost="p2p$((4-instance))"
    #bindhost="eth$((3+2*instance))"
    echo "binding $net_thread to $bindhost"
  fi

  echo numactl $numaops $membind       \
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

  numactl $numaops $membind            \
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
    -c $outcpu s6_output_thread        \
     < /dev/null                       \
    1> s6c${mys6cn}.out.$log_timestamp.$instance \
    2> s6c${mys6cn}.err.$log_timestamp.$instance &
}

# Start all instances
for instidx in ${instance_i[@]}
do
  args="${instances[$instidx]}"
  if [ -n "${args}" ]
  then
    echo
    echo Starting instance s6c$mys6cn/$instidx
    init $instidx $args
    echo Instance s6c$mys6cn/$instidx pid $!
    # Sleep to let instance come up
    sleep 10
  else
    echo Instance $instidx not defined for host $hostname
  fi
done

if [ $net_thread == 's6_pktsock_thread' ]
then
  # Zero out MISSEDPK counts
  for instidx in ${instance_i[@]}
  do
    for key in MISSEDPK NETDRPTL NETPKTTL
    do
      echo Resetting $key count for s6c$mys6cn/$instidx
      hashpipe_check_status -I $instidx -k $key -s 0
    done
  done
else
  # Zero out MISSPKTL counts
  for instidx in ${instance_i[@]}
  do
    echo Resetting MISSPKTL count for s6c$mys6cn/$instidx
    hashpipe_check_status -I $instidx -k MISSPKTL -s 0
  done

  # Release NETHOLD
  for instidx in ${instance_i[@]}
  do
    echo Releasing NETHOLD for s6c$mys6cn/$instidx
    hashpipe_check_status -I $instidx -k NETHOLD -s 0
  done
fi

# test mode
for instidx in ${instance_i[@]}
do
  echo Turning on TESTMODE for $mys6cn/$instidx
  hashpipe_check_status -I $instidx -k TESTMODE -s 0
done

# test mode
for instidx in ${instance_i[@]}
do
  echo Turning on RUNALWYS for $mys6cn/$instidx
  hashpipe_check_status -I RUNALWYS $instidx -k  -s 1
done

