#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <hiredis/hiredis.h>

#include "hashpipe.h"
#include "s6_obs_data_fast.h"

//----------------------------------------------------------
static redisContext * redis_connect(char *hostname, int port) {
//----------------------------------------------------------
    redisContext *c;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            hashpipe_error(__FUNCTION__, c->errstr);
            redisFree(c);   // get rid of the in-error context
            c = NULL;       // indicate error to caller (TODO - does redisFree null the context pointer?)
        } else {
            hashpipe_error(__FUNCTION__, "Connection error: can't allocate redis context");
        }
    }

    return(c);

}

//----------------------------------------------------------
static int s6_strcpy(char * dest, char * src, int strsize=FASTSTATUS_STRING_SIZE) {
//----------------------------------------------------------

    strncpy(dest, src, strsize);
    if(dest[strsize-1] != '\0') {
        dest[strsize-1] = '\0';
        hashpipe_error(__FUNCTION__, "FAST status string exceeded buffer size of %d, truncated : %s", strsize, dest);
    }
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
        hashpipe_error(__FUNCTION__, "redis query (%s) returned an error : %s", query, errstr);
    }

    return(rv); 
}

//----------------------------------------------------------
int put_obs_fast_info_to_redis(char * fits_filename, faststatus_t * faststatus, int instance, char *hostname, int port) {
//----------------------------------------------------------
    redisContext *c;
    redisReply *reply;
    char key[200];
    char time_str[200];
    char my_hostname[200];
    int rv=0;

    // TODO - sane rv

    // TODO make c static?
    c = redis_connect(hostname, port);
    if (!c) {
        rv = 1;
    }

#if 0
    if(!rv) {
        // update current filename
        // On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
        rv =  gethostname(my_hostname, sizeof(my_hostname));
        sprintf(key, "FN%s_%02d", my_hostname, instance);
        reply = (redisReply *)redisCommand(c,"SET %s %s", key, fits_filename);
        freeReplyObject(reply);
    }
#endif

    // TODO - possible race condition with FRB proccess
    if(!rv && faststatus->DUMPVOLT) {
        sprintf(time_str, "%ld", time(NULL));
        reply = (redisReply *)redisCommand(c,"MSET  %s %s %s", "DUMPRAW", time, "0");
        freeReplyObject(reply);
    }

    if(c) redisFree(c);       // TODO do I really want to free each time?

    return(rv);
}

//----------------------------------------------------------
int get_obs_fast_info_from_redis(faststatus_t * faststatus,     
                            char    *hostname, 
                            int     port) {
//----------------------------------------------------------

    redisContext *c;
    redisReply *reply;
    int rv = 0;

    double mjd_now;  

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    // TODO make c static?
    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            hashpipe_error(__FUNCTION__, c->errstr);
            redisFree(c);
        } else {
            hashpipe_error(__FUNCTION__, "Connection error: can't allocate redis context");
        }
        exit(1);
    }

    // make sure redis is being updated!
    // example from AO:
#if 0
    if(scram->AGCTIME == prior_agc_time) {
        no_time_change_count++;
        hashpipe_warn(__FUNCTION__, "agctime in redis databse has not been updated over %d queries", no_time_change_count);
        if(no_time_change_count >= no_time_change_limit) {
            hashpipe_error(__FUNCTION__, "redis databse is static!");
            rv = 1;
        }
    } else {
        no_time_change_count = 0;
        prior_agc_time = scram->AGCTIME;
    } 
#endif

    // Time
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET UNIXTIME      TIMETIM TIME"))) {
         faststatus->TIMETIM = atoi(reply->element[0]->str);
         faststatus->TIME    = atof(reply->element[1]->str);
         freeReplyObject(reply);
    } 

    // Receiver
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET REC      RECTIM RECEIVER"))) {
         faststatus->RECTIM =    atoi(reply->element[0]->str);
         strcpy(faststatus->RECEIVER, reply->element[1]->str);
         freeReplyObject(reply);
    } 

    // Telescope pointing
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET POINTING      POINTTIM POINTRA POINTDEC"))) {
         faststatus->POINTTIM = atoi(reply->element[0]->str);
         faststatus->POINTRA  = atof(reply->element[1]->str);
         faststatus->POINTDEC = atof(reply->element[2]->str);
         freeReplyObject(reply);
    } 

    // Clock synth
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET CLOCKSYN      CLOCKTIM CLOCKFRQ CLOCKDBM CLOCKLOC"))) {
        faststatus->CLOCKTIM = atoi(reply->element[0]->str);
        faststatus->CLOCKFRQ = atof(reply->element[1]->str);
        faststatus->CLOCKDBM = atof(reply->element[2]->str);
        faststatus->CLOCKLOC = atoi(reply->element[3]->str);
        freeReplyObject(reply);
    } 

    // Birdie synth
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET BIRDISYN      BIRDITIM BIRDIFRQ BIRDIDBM BIRDILOC"))) {
        faststatus->BIRDITIM = atoi(reply->element[0]->str);
        faststatus->BIRDIFRQ = atof(reply->element[1]->str);
        faststatus->BIRDIDBM = atof(reply->element[2]->str);
        faststatus->BIRDILOC = atoi(reply->element[3]->str);
        freeReplyObject(reply);
    } 

    // Raw data dump request
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET DUMPRAW      DUMPTIME DUMPVOLT"))) {
        faststatus->DUMPTIME = atoi(reply->element[0]->str);
        faststatus->DUMPVOLT = atof(reply->element[1]->str);
        freeReplyObject(reply);
    } 

    if(c) redisFree(c);       // TODO do I really want to free each time?

    return rv;         
}
