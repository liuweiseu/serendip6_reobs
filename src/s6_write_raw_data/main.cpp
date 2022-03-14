#include <stdio.h>
#include <string.h>
#include <time.h>
#include "include/s6_write_raw_data.h"
/*
These code is used for writing raw data test.
*/
int main()
{
    // create file name, based on linux time
    char filename[256];
    memset(filename,0,256);
    struct timespec time_spec;
    clock_gettime(CLOCK_REALTIME, &time_spec);
    printf("current time: %ld %ld\r\n",time_spec.tv_sec, time_spec.tv_nsec);
    struct tm tm_now;
    time_t time_now = (time_t)(time_spec.tv_sec);
    localtime_r(&time_now, &tm_now);
    printf("%d-%d-%d %d:%d:%d\r\n",tm_now.tm_year,tm_now.tm_mon,tm_now.tm_mday,tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    char packet_time[50];
    create_rawdata_filename("m15","1.05G-1.45G",19,1,time_spec.tv_sec,time_spec.tv_nsec,filename);
    printf("filename: %s\r\n",filename);
    
    // generate fake raw data, all ones
    char data[1024];
    memset(data,1,1024);
    
    // write raw data into file
    FILE *fp;
    fp = open_rawdata_file(filename);
    if(fp==NULL) printf("create file error\r\n");
    write_rawdata(data,1024,fp);
    printf("OK\r\n");
    close_rawdata_file(fp);
    return 0;

}