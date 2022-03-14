#ifndef _WRITE_RAW_DATA_H
#define _WRITE_RAW_DATA_H

#include <stdio.h>

void create_rawdata_filename(char *compute_node, char *freq_range, int bm_no, int pol,char* time_now, char *filename);
int write_rawdata(char *d, unsigned long long l,FILE* fp);
FILE* open_rawdata_file(char *filename);
void close_rawdata_file(FILE *fp);

#endif