#include <stdio.h>
#include <string.h>
#include "include/s6_write_raw_data.h"
/*
These code is used for writing raw data test.
*/
int main()
{
    // create file name, based on linux time
    char filename[100];
    memset(filename,0,100);
    create_rawdata_filename("m15","1.05G-1.45G",19,1,"2022-03-11",filename);
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