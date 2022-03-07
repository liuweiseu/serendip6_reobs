#include <stdio.h>
#include <string.h>
#include "include/s6_write_raw_data.h"

int main()
{
    char filename[100];
    create_file_name(filename);
    printf("length: %ld\r\n",strlen(filename));
    printf("filename: %s\r\n",filename);
    
    char data[1024];
    memset(data,1,1024);
    
    FILE *fp;
    fp = fopen(filename,"w");
    if(fp==NULL) printf("create file error\r\n");
    //write_raw_data(data,1024,fp);
    fwrite(data,1024,1,fp);
    printf("OK\r\n");
    fclose(fp);
    return 0;

}