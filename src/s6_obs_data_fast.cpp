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
#define MAX_STRING_LENGTH 32  
#define MAX_TOKENS         4
int tokenize_string(char * &pInputString, char * Delimiter, char * pToken[MAX_TOKENS]) {
//----------------------------------------------------------
  int i=0;

  pToken[i] = strtok(pInputString, Delimiter);
  i++;

  while ((pToken[i] = strtok(NULL, Delimiter)) != NULL){
    i++;
	if(i >= MAX_TOKENS) {
		i = -1;
		break;
	}
  }

  return i;
}

//----------------------------------------------------------
int coord_string_to_decimal(char * &coord_string, double * coord_decimal) {
//----------------------------------------------------------
// Takes string of the form DH:MM:SS, including any sign, and
// returns decimal degrees or hours.  DH can be degrees or hours.

	char * pTokens[MAX_TOKENS];
	int rv;

	rv = tokenize_string(coord_string, ":", pTokens);
	if(rv == 3) {
		*coord_decimal = (atof(pTokens[0]) + atof(pTokens[1])/60.0 + atof(pTokens[2])/3600.0);
		rv = 0;
	} else {
        hashpipe_error(__FUNCTION__, "Malformed coordinate string : %s", coord_string);
		rv = 1;
	}

	return(rv);
}

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
    redisContext *c_observatory;
    redisReply *reply;
    char key[200];
    char time_str[200];
    char my_hostname[200];
    int rv=0;

	const char * host_observatory = "10.128.8.8";
	int port_observatory = 6379;

    // TODO - sane rv

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
    redisContext *c_observatory;
    redisReply *reply;
    int rv = 0;

	const char * host_observatory = "10.128.8.8";
	int port_observatory = 6379;

    char computehostname[32];
    char query_string[64];

    double mjd_now;  

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

	// Local instrument DB
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

#if 0
	// Observatory DB
    c_observatory = redisConnectWithTimeout((char *)host_observatory, port_observatory, timeout);
    if (c == NULL || c->err) {
        if (c) {
            hashpipe_error(__FUNCTION__, c->errstr);
            redisFree(c);
        } else {
            hashpipe_error(__FUNCTION__, "Connection error: can't allocate redis context");
        }
        exit(1);
    }
	rv = s6_redis_get(c_observatory, &reply,"select 2");
#endif

	gethostname(computehostname, sizeof(computehostname));

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

#if 0
    // Time
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET UNIXTIME      TIMETIM TIME"))) {
         faststatus->TIMETIM = atoi(reply->element[0]->str);
         faststatus->TIME    = atof(reply->element[1]->str);
         freeReplyObject(reply);
    } 
#endif

    // Receiver
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET REC      RECTIM RECEIVER"))) {
         faststatus->RECTIM =    atoi(reply->element[0]->str);
         strcpy(faststatus->RECEIVER, reply->element[1]->str);
         freeReplyObject(reply);
    } 

#if 1
    // Telescope pointing
	sprintf(query_string, "HMGET       PNT_%s       PNTTIME PNTRA PNTDEC", computehostname);
    if(!rv && !(rv = s6_redis_get(c, &reply, query_string))) {
         faststatus->POINTTIM = atoi(reply->element[0]->str);
         faststatus->POINTRA  = atof(reply->element[1]->str);
         faststatus->POINTDEC = atof(reply->element[2]->str);
         freeReplyObject(reply);
    } 

#endif
    // Clock synth
    if(!rv && !(rv = s6_redis_get(c, &reply,"HMGET CLOCKSYN      CLOCKTIM CLOCKFRQ CLOCKDBM CLOCKLOC"))) {
        faststatus->CLOCKTIM = atoi(reply->element[0]->str);
        faststatus->CLOCKFRQ = atof(reply->element[1]->str);
        faststatus->CLOCKDBM = atof(reply->element[2]->str);
        faststatus->CLOCKLOC = atoi(reply->element[3]->str);
        freeReplyObject(reply);
    } 

    // ADC RMS's
	sprintf(query_string, "HMGET       ADCRMS_%s       ADCRMSTM ADCRMSP0 ADCRMSP1", computehostname);
    if(!rv && !(rv = s6_redis_get(c, &reply, query_string))) {
        faststatus->ADCRMSTM = atoi(reply->element[0]->str);
        faststatus->ADCRMSP0 = atof(reply->element[1]->str);
        faststatus->ADCRMSP1 = atof(reply->element[2]->str);
        freeReplyObject(reply);
    } 

#if 0
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
#endif

	// Get observatory data
	
#if 0
	// observtory data timestamp
	if(!rv && !(rv = s6_redis_get(c_observatory, &reply,"hmget ZK_KY_DATA Timestamp"))) {
		faststatus->ZKDTIME = atof(reply->element[0]->str)/1000.0;	// millisec to sec
	}

	// coordinates
	if(!rv) rv = s6_redis_get(c_observatory, &reply,"hmget ZK_COORDINATE Equator_T_RA Equator_T_DEC");
	if(!rv)	rv = coord_string_to_decimal(reply->element[0]->str, &(faststatus->EQTRA));
	if(!rv) rv = coord_string_to_decimal(reply->element[1]->str, &(faststatus->EQDDEC));
#endif
   
    if(c) redisFree(c);       // TODO do I really want to free each time?
//    if(c_observatory) redisFree(c_observatory);       // TODO do I really want to free each time?

    return rv;         
}
