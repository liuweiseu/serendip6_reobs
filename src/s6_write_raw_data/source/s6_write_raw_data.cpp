#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../include/s6_write_raw_data.h"

static FILE *fp;
static void str_replace(char *s, char c, char r)
{
    int l = strlen(s);
    for(int i = 0; i < l; i++)
        if(s[i]==c)s[i] = r;
}

void create_rawdata_filename(char *compute_node, char *freq_range, int bm_no, int pol,unsigned long time_sec, unsigned long time_nsec, char *filename)
{
    // convert the time 
    struct tm tm_now;
    time_t time_now = (time_t)(time_sec);
    localtime_r(&time_now, &tm_now);

    sprintf(filename,"serendip6_%s_%s_MB_%02d_%02d_%04d%02d%02d_%02d%02d%02d_%09ld_raw.dat",
                                compute_node,
                                freq_range,
                                bm_no,
                                pol,
                                tm_now.tm_year+1900,
                                tm_now.tm_mon+1,
                                tm_now.tm_mday,
                                tm_now.tm_hour,
                                tm_now.tm_min,
                                tm_now.tm_sec,
                                time_nsec);
}

int write_rawdata(double *d, unsigned long long l)
{
    fwrite(d,l,sizeof(double),fp);
    return 0;
}

int open_rawdata_file(char *filename)
{
    fp = fopen(filename,"w");
    if(fp==NULL)
        printf("file can't be opened.\r\n");
    return 0;
}

void close_rawdata_file()
{
    fclose(fp);
}