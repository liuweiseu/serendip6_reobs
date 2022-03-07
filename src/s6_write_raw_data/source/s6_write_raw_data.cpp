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

void create_file_name(char *filename)
{
    // get the current time
    time_t t;
    time(&t);
    // replace space to '-'
    memcpy(filename,ctime(&t),strlen(ctime(&t))+1);
    str_replace(filename,' ','-');
    str_replace(filename,'\n',0);
    strcat(filename,".dat");
}

int write_raw_data(char *d, unsigned long long l, FILE *fp)
{
    fwrite(d,l,1,fp);
    return 0;
}