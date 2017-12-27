#ifndef _S6_OBS_DATA_FAST_H
#define _S6_OBS_DATA_FAST_H

#include "s6_databuf.h"

#define N_ADCS_PER_ROACH2 8     // should this be N_BEAM_SLOTS/2 ?

#define FASTSTATUS_STRING_SIZE 32
#define FASTSTATUS_BIG_STRING_SIZE 256

#define CURRENT_MJD ((time(NULL) / 86400.0 ) + 40587.0)             // 40587.0 is the MJD of the unix epoch

// idle status reasons bitmap
#define idle_nibble_01_1bit                     0x000000000000001; // saving these 4 for something important
#define idle_nibble_01_2bit                     0x000000000000002;
#define idle_nibble_01_4bit                     0x000000000000004;
#define idle_nibble_01_8bit                     0x000000000000008;



typedef struct faststatus {

   char RECEIVER[FASTSTATUS_STRING_SIZE];       // receiver
   long RECEIVERSTIME;

   int     CLOCKTIM;
   double  CLOCKFRQ;
   double  CLOCKDBM;
   int     CLOCKLOC;
   int     BIRDITIM;
   double  BIRDIFRQ;
   double  BIRDIDBM;
   int     BIRDILOC;

  int     coarse_chan_id;
} faststatus_t;

int get_obs_fast_info_from_redis(faststatus_t *faststatus, char *hostname, int port);
int put_obs_fast_info_to_redis(char * fits_filename, int instance, char *hostname, int port);

#endif  // _S6_OBS_DATA_FAST_H

