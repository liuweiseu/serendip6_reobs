#! /bin/bash

echo BEFORE
echo "/proc/sys/net/ipv4/conf/p2p1/force_igmp_version " `cat /proc/sys/net/ipv4/conf/p2p1/force_igmp_version`" (should be 2)"
echo "/proc/sys/net/ipv4/conf/p2p2/force_igmp_version " `cat /proc/sys/net/ipv4/conf/p2p2/force_igmp_version`" (should be 2)"
echo "/proc/sys/net/ipv4/conf/p2p3/force_igmp_version " `cat /proc/sys/net/ipv4/conf/p2p3/force_igmp_version`" (should be 2)"
echo "/proc/sys/net/ipv4/conf/p2p4/force_igmp_version " `cat /proc/sys/net/ipv4/conf/p2p4/force_igmp_version`" (should be 2)"
echo "/proc/sys/net/ipv4/conf/all/rp_filter           " `cat /proc/sys/net/ipv4/conf/all/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/p2p1/rp_filter          " `cat /proc/sys/net/ipv4/conf/p2p1/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/p2p2/rp_filter          " `cat /proc/sys/net/ipv4/conf/p2p2/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/p2p3/rp_filter          " `cat /proc/sys/net/ipv4/conf/p2p3/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/p2p4/rp_filter          " `cat /proc/sys/net/ipv4/conf/p2p4/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/all/arp_ignore          " `cat /proc/sys/net/ipv4/conf/all/arp_ignore`" (should be 1)"
echo "/proc/sys/net/ipv4/conf/all/arp_announce        " `cat /proc/sys/net/ipv4/conf/all/arp_announce`" (should be 2)"
echo " "

echo Making modifications
# force IGMPv2 for all network interfaces need multicast
sudo echo 2 > /proc/sys/net/ipv4/conf/p2p1/force_igmp_version
sudo echo 2 > /proc/sys/net/ipv4/conf/p2p2/force_igmp_version
sudo echo 2 > /proc/sys/net/ipv4/conf/p2p3/force_igmp_version
sudo echo 2 > /proc/sys/net/ipv4/conf/p2p4/force_igmp_version
# Disable reverse path filter
sudo echo 0 > /proc/sys/net/ipv4/conf/all/rp_filter
sudo echo 0 > /proc/sys/net/ipv4/conf/p2p1/rp_filter
sudo echo 0 > /proc/sys/net/ipv4/conf/p2p2/rp_filter
sudo echo 0 > /proc/sys/net/ipv4/conf/p2p3/rp_filter
sudo echo 0 > /proc/sys/net/ipv4/conf/p2p4/rp_filter
# Disable the so called "ARP flux" 
sudo echo 1 > /proc/sys/net/ipv4/conf/all/arp_ignore
sudo echo 2 > /proc/sys/net/ipv4/conf/all/arp_announce
echo " "

echo AFTER
echo "/proc/sys/net/ipv4/conf/p2p1/force_igmp_version " `cat /proc/sys/net/ipv4/conf/p2p1/force_igmp_version`" (should be 2)"
echo "/proc/sys/net/ipv4/conf/p2p2/force_igmp_version " `cat /proc/sys/net/ipv4/conf/p2p2/force_igmp_version`" (should be 2)"
echo "/proc/sys/net/ipv4/conf/p2p3/force_igmp_version " `cat /proc/sys/net/ipv4/conf/p2p3/force_igmp_version`" (should be 2)"
echo "/proc/sys/net/ipv4/conf/p2p4/force_igmp_version " `cat /proc/sys/net/ipv4/conf/p2p4/force_igmp_version`" (should be 2)"
echo "/proc/sys/net/ipv4/conf/all/rp_filter           " `cat /proc/sys/net/ipv4/conf/all/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/p2p1/rp_filter          " `cat /proc/sys/net/ipv4/conf/p2p1/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/p2p2/rp_filter          " `cat /proc/sys/net/ipv4/conf/p2p2/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/p2p3/rp_filter          " `cat /proc/sys/net/ipv4/conf/p2p3/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/p2p4/rp_filter          " `cat /proc/sys/net/ipv4/conf/p2p4/rp_filter`" (should be 0)"         
echo "/proc/sys/net/ipv4/conf/all/arp_ignore          " `cat /proc/sys/net/ipv4/conf/all/arp_ignore`" (should be 1)"
echo "/proc/sys/net/ipv4/conf/all/arp_announce        " `cat /proc/sys/net/ipv4/conf/all/arp_announce`" (should be 2)"
echo " "
