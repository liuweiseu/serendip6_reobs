#ifndef _REDIS_H
#define _REDIS_H

#include <stdio.h>

#define HOST    "127.0.0.1"
#define PORT    6379

int get_info_from_redis(char *hostname, int port, const char *list, const char *key, char *value);
int put_info_to_redis(char *hostname, int port, const char *key, const char *value);
void create_metadata_filename(char *filename);
FILE* open_metadata_file(char *filename);
void write_metadata(char *d, FILE *fp);
void close_metadata_file(FILE *fp);

#endif