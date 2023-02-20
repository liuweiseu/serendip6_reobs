#! /bin/bash

cp serendip6_reobs.so /usr/local/lib
cp CudaPFB_Lib/libcudapfb.so /usr/local/lib
 
ls -l /usr/local/lib |grep reobs
ls -l /usr/local/lib |grep cudapfb
