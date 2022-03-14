#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../include/s6_write_raw_data.h"

static void str_replace(char *s, char c, char r)
{
    int l = strlen(s);
    for(int i = 0; i < l; i++)
        if(s[i]==c)s[i] = r;
}

void create_rawdata_filename(char *compute_node, char *freq_range, int bm_no, int pol,char* time_now,char *filename)
{
    char bm_no_str[4];
    char pol_str[4];
    memset(bm_no_str,0,4);
    memset(pol_str,0,4);
    sprintf(bm_no_str,"%d",bm_no);
    sprintf(pol_str,"%d",pol);
    strcat(filename, "serendip6");       // add "serendip6" to the filename
    strcat(filename, "_");
    strcat(filename, compute_node);      // add compute node to the filename
    strcat(filename, "_");
    strcat(filename, freq_range);        // add frequency to the filename
    strcat(filename, "_");
    strcat(filename, "MB");              // add multi beam symbol to the filename
    strcat(filename, "_");
    strcat(filename, bm_no_str);         // add bm number to the filename
    strcat(filename, "_");
    strcat(filename, pol_str);           // add pol to the filename
    strcat(filename, "_");
    strcat(filename, time_now);          // add current time to the filename 
    strcat(filename, "_");
    strcat(filename,"raw.dat");
}

int write_rawdata(char *d, unsigned long long l, FILE *fp)
{
    fwrite(d,l,1,fp);
    return 0;
}

FILE* open_rawdata_file(char *filename)
{
    return fopen(filename,"w");
}

void close_rawdata_file(FILE *fp)
{
    fclose(fp);
}