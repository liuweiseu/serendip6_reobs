#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <hiredis/hiredis.h>
//#include "hashpipe.h"

#include "../include/s6_redis.h"
//----------------------------------------------------------
static redisContext * redis_connect(char *hostname, int port) {
//----------------------------------------------------------
    redisContext *c;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            fprintf(stderr, "%s\r\n", c->errstr);
            redisFree(c);   // get rid of the in-error context
            c = NULL;       // indicate error to caller (TODO - does redisFree null the context pointer?)
        } else {
            fprintf(stderr, "%s\r\n", "Connection error: can't allocate redis context");
        }
    }
    return(c);
}

//----------------------------------------------------------
static int s6_redis_get(redisContext *c, redisReply ** reply, const char * query) {
//----------------------------------------------------------

    int rv = 0;
    int i;
    char * errstr;

    *reply = (redisReply *)redisCommand(c, query);

    if(*reply == NULL) {
        errstr = c->errstr;
        rv = 1;
    } else if((*reply)->type == REDIS_REPLY_ERROR) {
        errstr = (*reply)->str;
        rv = 1;
    } else if((*reply)->type == REDIS_REPLY_ARRAY) {
        for(i=0; i < (*reply)->elements; i++) {
            if(!(*reply)->element[i]->str) {
                errstr = (char *)"At least one element in the array was empty";
                rv = 1;
                break;
            }
        }
    }

    if(rv) {
        fprintf(stderr, "redis query (%s) returned an error : %s\r\n", query, errstr);
    }
    return(rv); 
}

//----------------------------------------------------------
int put_info_to_redis(char *hostname, int port, const char * key, const char * value) {
//----------------------------------------------------------
    redisContext *c;
    redisReply *reply;
    int rv=0;
    /*
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);

    if(c) {
        reply = (redisReply *)redisCommand(c,"SET  %s %s", key, value);
        freeReplyObject(reply);
        redisFree(c); 
    }
    */
    return(rv);
}

//----------------------------------------------------------
int get_info_from_redis(char *hostname, int port, const char* key, const char * field, char *value) {
//----------------------------------------------------------

    redisContext *c;
    int rv = 0;
    redisReply *reply;

    char query[64];
    memset(query, 0 ,64);
    sprintf(query, "hmget %s %s", key, field);
    
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	// Local instrument DB
    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            fprintf(stderr, "%s\r\n", c->errstr);
            redisFree(c);
        } else {
            fprintf(stderr, "Connection error: can't allocate redis context");
        }
        exit(1);
    }

    rv = s6_redis_get(c, &reply,"AUTH fast");

	rv = s6_redis_get(c, &reply, query);
    memcpy(value, reply->element[0]->str,reply->element[0]->len);
    freeReplyObject(reply);
    if(c) redisFree(c);
    
    return rv;         
}


void create_metadata_filename(char *filename)
{

}

FILE* open_metadata_file(char *filename)
{
    return NULL;
}

void write_metadata(char *d, FILE *fp)
{
    
}

void close_metadata_file(FILE *fp)
{
    
}
