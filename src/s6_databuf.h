#ifndef _S6_DATABUF_H
#define _S6_DATABUF_H

#include <stdint.h>
#include <cufft.h>
#include "hashpipe_databuf.h"

#include "CudaPFB_Lib/CudaPFB.h"
#define SOURCE_FAST 1
#define PAGE_SIZE               (4096)
#define CACHE_ALIGNMENT         (256)
// TODO
// SMOOTH_SCALE, POWER_THRESH, MAXHITS, and MAXGPUHITS should be input parms
// N_FINE_CHAN and N_COARSE_CHAN - should they be input parms?

// N_COARSE_CHAN needs to be evenly divisible by 32 in order for each time sample
// ("row" of coarse chans) to be 256 bit aligned.  256 bit alignment is required by
// the "non-temporal" memory copy. 
//#define N_POLS_PER_BEAM         2
//#define N_BYTES_PER_SAMPLE      2

#ifdef SOURCE_S6
// channelized complex input
#define N_POLS_PER_BEAM             2
#define N_BYTES_PER_SAMPLE      	2
#define N_BEAMS                     7
#define N_BEAM_SLOTS                8
#define N_COARSE_CHAN               320 
#define N_TIME_SAMPLES              ((uint64_t)128*1024)               
#define N_FINE_CHAN					N_TIME_SAMPLES                
#define N_SPECTRA_PER_PACKET        1
#define N_SUBSPECTRA_PER_SPECTRUM   1
#define N_SAMPLES_PER_BLOCK         (N_FINE_CHAN * N_COARSE_CHAN * N_POLS_PER_BEAM * N_BEAM_SLOTS)
#define N_BORS                      (N_BEAMS)
#define N_SOURCE_NODES              (N_BEAMS)
#define N_COARSE_CHAN_PER_BORS      (N_COARSE_CHAN)
#define SMOOTH_SCALE                1024
#define N_INPUT_BLOCKS          3
#define N_DEBUG_INPUT_BLOCKS    0
#define N_OUTPUT_BLOCKS         3

#elif SOURCE_DIBAS
// channelized complex input
#define N_POLS_PER_BEAM             2
#define N_BYTES_PER_SAMPLE      	2
#define N_BEAMS                     1
#define N_BEAM_SLOTS                1
#define N_COARSE_CHAN               512 
#define N_TIME_SAMPLES              ((uint64_t) 512*1024)               
#define N_FINE_CHAN 				N_TIME_SAMPLES               
#define N_SPECTRA_PER_PACKET        4
#define N_SUBSPECTRA_PER_SPECTRUM   8
#define N_SAMPLES_PER_BLOCK         (N_FINE_CHAN * N_COARSE_CHAN * N_POLS_PER_BEAM)
#define N_BORS                      (N_SUBSPECTRA_PER_SPECTRUM)
#define N_SOURCE_NODES              8
#define N_COARSE_CHAN_PER_BORS      (N_COARSE_CHAN/N_BORS)
#define SMOOTH_SCALE                1024
#define N_INPUT_BLOCKS              3
#define N_DEBUG_INPUT_BLOCKS        0
#define N_OUTPUT_BLOCKS             3

#elif SOURCE_FAST
// non-channelized real input
#define N_POLS_PER_BEAM             1
#define N_POLS_PER_BEAM_TOTAL       2
#define N_BYTES_PER_SAMPLE      	  1
#define N_BEAMS                     19
#define N_BEAM_SLOTS                1
#define N_COARSE_CHAN               1             
#define N_TIME_SAMPLES              ((uint64_t) 256*1024*1024)     //*1024*1024                       
#define N_FINE_CHAN 				        (N_TIME_SAMPLES/2)                               
#define N_SPECTRA_PER_PACKET        4096
#define N_SUBSPECTRA_PER_SPECTRUM   1
#define N_SAMPLES_PER_BLOCK         (N_TIME_SAMPLES * N_COARSE_CHAN * N_POLS_PER_BEAM)
#define N_BORS                      (N_SUBSPECTRA_PER_SPECTRUM)
#define N_SOURCE_NODES              (N_BEAMS * N_POLS_PER_BEAM_TOTAL)
#define N_COARSE_CHAN_PER_BORS      (N_COARSE_CHAN/N_BORS)
#define SMOOTH_SCALE                (8*1024)
#define N_INPUT_BLOCKS          24   //24
#define N_DEBUG_INPUT_BLOCKS    0
#define N_OUTPUT_BLOCKS         8
// for FAST, the clock frequency is hard set to 1GHz, expressed here in MHz
#define CLOCK_FREQ		1000.0
#endif

#define N_SAMPLES_PER_BEAM      (N_FINE_CHAN * N_COARSE_CHAN * N_POLS_PER_BEAM)
#define N_BYTES_PER_BEAM        (N_BYTES_PER_SAMPLE * N_SAMPLES_PER_BEAM)
#define N_BYTES_PER_SUBSPECTRUM (N_BYTES_PER_SAMPLE * N_COARSE_CHAN / N_SUBSPECTRA_PER_SPECTRUM * N_POLS_PER_BEAM)
#define N_DATA_BYTES_PER_BLOCK  (N_BYTES_PER_SAMPLE * N_SAMPLES_PER_BLOCK)

#define POWER_THRESH            20.0
#define MIN_POWER_THRESH        10.0
#define MAXGPUHITS              ((int)(1.0 / MIN_POWER_THRESH * N_FINE_CHAN))    
#define MAXHITS                 4096

#define N_BYTES_PER_OUT_SAMPLE  sizeof(float)
#define N_DATA_BYTES_PER_OUT_BLOCK OUTPUT_LEN * N_BYTES_PER_OUT_SAMPLE
//#define FREQ_RANGE              "1.05G-1.45G"
// The following 3 #define's are needed only by s6_gen_fake_data.
// Perhaps they should be removed at some point (with a change to
// s6_gen_fake_data).
#define N_SUBBANDS              1
#define N_SUBBAND_CHAN          (N_COARSE_CHAN / N_SUBBANDS)
//#define N_GPU_ELEMENTS          (N_FINE_CHAN * N_COARSE_CHAN_PER_BORS)

// Used to pad after hashpipe_databuf_t to maintain cache alignment
typedef uint8_t hashpipe_databuf_cache_alignment[
  CACHE_ALIGNMENT - (sizeof(hashpipe_databuf_t)%CACHE_ALIGNMENT)
];

/*
 * INPUT BUFFER STRUCTURES
 */
typedef struct s6_input_block_header {
  uint64_t mcnt;                    // mcount of first packet
  uint64_t coarse_chan_id;          // coarse channel number of lowest channel in this block
  uint64_t num_coarse_chan;         // number of actual coarse channels (<= N_COARSE_CHAN)
  uint64_t sid;                     // source ID
  uint64_t time_sec;				        // unix time that this block was marked complete
  uint64_t time_nsec;				        // nanoseconds past time_sec
  uint64_t missed_pkts[N_BORS];     // missed per beam - this block or this run? TODO
} s6_input_block_header_t;

typedef uint8_t s6_input_header_cache_alignment[
  CACHE_ALIGNMENT - (sizeof(s6_input_block_header_t)%CACHE_ALIGNMENT)
];

typedef struct s6_input_block {
  s6_input_block_header_t header;
  s6_input_header_cache_alignment padding; // Maintain cache alignment
  uint64_t data[(N_DATA_BYTES_PER_BLOCK/sizeof(uint64_t))];
} s6_input_block_t;

typedef struct s6_input_databuf {
  hashpipe_databuf_t header;
  hashpipe_databuf_cache_alignment padding; // Maintain cache alignment   
  s6_input_block_t block[N_INPUT_BLOCKS];
} s6_input_databuf_t;

/*
 * OUTPUT BUFFER STRUCTURES
 */
typedef struct s6_output_block_header {
  uint64_t mcnt;
  uint64_t coarse_chan_id;          // coarse channel number of lowest channel in this block
  uint64_t num_coarse_chan;         // number of actual coarse channels (<= N_COARSE_CHAN)
#ifdef SOURCE_FAST
  uint64_t sid;                     // source ID, beampol for FAST
#endif
  uint64_t time_sec;				        // unix time that this block (on the input side) was marked complete
  uint64_t time_nsec;				        // nanoseconds past time_sec
  uint64_t missed_pkts[N_BORS];     // missed per beam - this block or this run? TODO
  uint64_t nhits[N_BORS];
} s6_output_block_header_t;

typedef uint8_t s6_output_header_cache_alignment[
  CACHE_ALIGNMENT - (sizeof(s6_output_block_header_t)%CACHE_ALIGNMENT)
];

typedef struct s6_output_block {
  s6_output_block_header_t header;
  s6_output_header_cache_alignment padding; // Maintain cache alignment
  char data[(OUTPUT_LEN)]; 
} s6_output_block_t;

typedef struct s6_output_databuf {
  hashpipe_databuf_t header;
  hashpipe_databuf_cache_alignment padding; // Maintain cache alignment   
  s6_output_block_t block[N_OUTPUT_BLOCKS];
} s6_output_databuf_t;

/*
 * INPUT BUFFER FUNCTIONS
 */
hashpipe_databuf_t *s6_input_databuf_create(int instance_id, int databuf_id);

static inline s6_input_databuf_t *s6_input_databuf_attach(int instance_id, int databuf_id)
{
    return (s6_input_databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int s6_input_databuf_detach(s6_input_databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline void s6_input_databuf_clear(s6_input_databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline int s6_input_databuf_block_status(s6_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int s6_input_databuf_total_status(s6_input_databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

int s6_input_databuf_wait_free(s6_input_databuf_t *d, int block_id);

int s6_input_databuf_busywait_free(s6_input_databuf_t *d, int block_id);

int s6_input_databuf_wait_filled(s6_input_databuf_t *d, int block_id);

int s6_input_databuf_busywait_filled(s6_input_databuf_t *d, int block_id);

int s6_input_databuf_set_free(s6_input_databuf_t *d, int block_id);

int s6_input_databuf_set_filled(s6_input_databuf_t *d, int block_id);

/*
 * OUTPUT BUFFER FUNCTIONS
 */

hashpipe_databuf_t *s6_output_databuf_create(int instance_id, int databuf_id);

static inline void s6_output_databuf_clear(s6_output_databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline s6_output_databuf_t *s6_output_databuf_attach(int instance_id, int databuf_id)
{
    return (s6_output_databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int s6_output_databuf_detach(s6_output_databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline int s6_output_databuf_block_status(s6_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int s6_output_databuf_total_status(s6_output_databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

static inline int s6_output_databuf_wait_free(s6_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int s6_output_databuf_busywait_free(s6_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int s6_output_databuf_wait_filled(s6_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int s6_output_databuf_busywait_filled(s6_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int s6_output_databuf_set_free(s6_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

static inline int s6_output_databuf_set_filled(s6_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}

#endif // _S6_DATABUF_H
