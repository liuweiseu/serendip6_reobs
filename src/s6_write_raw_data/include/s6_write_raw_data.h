#ifndef _WRITE_RAW_DATA_H
#define _WRITE_RAW_DATA_H

#include <stdio.h>

void create_file_name(char *filename);
int write_raw_data(char *d, unsigned long long l,FILE* fp);

static inline void openfile(char *filename, FILE *fp)
{
    fp = fopen(filename,"w");
}

static inline void closefile(FILE *fp)
{
    fclose(fp);
}
#endif