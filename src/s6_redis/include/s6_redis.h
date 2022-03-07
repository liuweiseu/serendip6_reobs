#ifndef _REDIS_H
#define _REDIS_H

#include <stdio.h>

#define HOST    "127.0.0.1"
#define PORT    6379

#define N_ADCS_PER_ROACH2 8     // should this be N_BEAM_SLOTS/2 ?

#define FASTSTATUS_STRING_SIZE 32
#define FASTSTATUS_BIG_STRING_SIZE 256

#define CURRENT_MJD ((time(NULL) / 86400.0 ) + 40587.0)             // 40587.0 is the MJD of the unix epoch

// idle status reasons bitmap
#define idle_nibble_01_1bit                     0x000000000000001; // saving these 4 for something important
#define idle_nibble_01_2bit                     0x000000000000002;
#define idle_nibble_01_4bit                     0x000000000000004;
#define idle_nibble_01_8bit                     0x000000000000008;

#define CLOCK_FREQ                              1000.0

typedef struct faststatus {

//   time_t   TIME;		// unix time, seconds
   double   TIMEFRAC;		// fractional seconds beyond TIME
   double   DUT1;		// current UT1 - UTC difference that is being broadcast by NIST

   char     RECEIVER[FASTSTATUS_STRING_SIZE];  

   double   PHAPOSX;
   double   PHAPOSY;
   double   PHAPOSZ;
   double   ANGLEM;

   double   POINTRA[19]; 
   double   POINTDEC[19];

   double   CLOCKFRQ;

   int	    ADCRMSTM;							// ADC RMS's 
   double   ADCRMSP0;							// polarization 0
   double   ADCRMSP1;							// polarization 1

   int      DUMPTIME;
   int      DUMPVOLT;

   int       coarse_chan_id;                       // will always be 0 for FAST (not coarse channelized)
} faststatus_t;

int get_info_from_redis(char *hostname, int port, const char *key, char *value);
int put_info_to_redis(char *hostname, int port, const char *key, const char *value);
void create_metadata_filename(char *filename);
FILE* open_metadata_file(char *filename);
void write_metadata(char *d, FILE *fp);
void close_metadata_file(FILE *fp);

#endif