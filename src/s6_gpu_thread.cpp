/*
 * s6_gpu_thread.c
 *
 * Performs spectroscopy of incoming data using s6GPU
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sched.h>

/*
#include <cuda.h>
#include <cufft.h>
#include <cuda_runtime_api.h>

#include <s6GPU.h>
*/
#include "hashpipe.h"
#include "s6_databuf.h"
//#include "s6GPU.h"

#define ELAPSED_NS(start,stop) \
  (((int64_t)stop.tv_sec-start.tv_sec)*1000*1000*1000+(stop.tv_nsec-start.tv_nsec))

static void *run(hashpipe_thread_args_t * args)
{
    printf("gpu_thread_V0.1\r\n");
    // Local aliases to shorten access to args fields
    s6_input_databuf_t *db_in = (s6_input_databuf_t *)args->ibuf;
    s6_output_databuf_t *db_out = (s6_output_databuf_t *)args->obuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;

#ifdef DEBUG_SEMS
    fprintf(stderr, "s/tid %lu/                      GPU/\n", pthread_self());
#endif

    int rv;
    uint64_t start_mcount, last_mcount=0;
    int s6gpu_error = 0;
    int curblock_in=0;
    int curblock_out=0;
    int error_count = 0, max_error_count = 0;
    float error, max_error = 0.0;

    struct timespec start, stop;
    uint64_t elapsed_gpu_ns  = 0;
    uint64_t gpu_block_count = 0;

    // init s6GPU
    int gpu_dev=0;          			// default to 0
    int maxhits = MAXHITS; 				// default
	float power_thresh = POWER_THRESH;	// default
    hashpipe_status_lock_safe(&st);
    hgeti4(st.buf, "GPUDEV", &gpu_dev);
    hgeti4(st.buf, "MAXHITS", &maxhits);
    hgetr4(st.buf, "POWTHRSH", &power_thresh);
    hputr4(st.buf, "POWTHRSH", power_thresh);
    hashpipe_status_unlock_safe(&st);
    //init_device(gpu_dev);
	char gpu_sem_name[256];
	sem_t * gpu_sem;
	sprintf(gpu_sem_name, "serendip6_gpu_sem_device_%d", gpu_dev);
	gpu_sem = sem_open(gpu_sem_name, O_CREAT, S_IRWXU, 1);
	
    
    uint64_t num_coarse_chan = N_COARSE_CHAN;

    while (run_threads()) {

        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "GPUBLKIN", curblock_in);
        hputs(st.buf, status_key, "waiting");
        hputi4(st.buf, "GPUBKOUT", curblock_out);
        hashpipe_status_unlock_safe(&st);

        // Wait for new input block to be filled
        while ((rv=hashpipe_databuf_wait_filled((hashpipe_databuf_t *)db_in, curblock_in)) != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for filled databuf");
                pthread_exit(NULL);
                break;
            }
        }

        // Got a new data block, update status and determine how to handle it
        hashpipe_status_lock_safe(&st);
        hputu8(st.buf, "GPUMCNT", db_in->block[curblock_in].header.mcnt);
        hashpipe_status_unlock_safe(&st);

        if(db_in->block[curblock_in].header.mcnt >= last_mcount) {
          // Wait for new output block to be free
          while ((rv=s6_output_databuf_wait_free(db_out, curblock_out)) != HASHPIPE_OK) {
              if (rv==HASHPIPE_TIMEOUT) {
                  hashpipe_status_lock_safe(&st);
                  hputs(st.buf, status_key, "blocked gpu out");
                  hashpipe_status_unlock_safe(&st);
                  continue;
              } else {
                  hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                  pthread_exit(NULL);
                  break;
              }
          }
        }

        // Note processing status
        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "processing gpu");
        hashpipe_status_unlock_safe(&st);

        clock_gettime(CLOCK_MONOTONIC, &start);

        // pass input metadata to output
        db_out->block[curblock_out].header.mcnt            = db_in->block[curblock_in].header.mcnt;
        db_out->block[curblock_out].header.coarse_chan_id  = db_in->block[curblock_in].header.coarse_chan_id;
        db_out->block[curblock_out].header.num_coarse_chan = db_in->block[curblock_in].header.num_coarse_chan;
        db_out->block[curblock_out].header.time_sec        = db_in->block[curblock_in].header.time_sec;
        db_out->block[curblock_out].header.time_nsec       = db_in->block[curblock_in].header.time_nsec;
        memcpy(&db_out->block[curblock_out].header.missed_pkts, 
               &db_in->block[curblock_in].header.missed_pkts, 
               sizeof(uint64_t) * N_BEAM_SLOTS);

        db_out->block[curblock_out].header.sid = db_in->block[curblock_in].header.sid;

        size_t total_hits = 0;
        if(num_coarse_chan) {
            int n_bors = N_SUBSPECTRA_PER_SPECTRUM;
            uint64_t n_bytes_per_bors  = N_BYTES_PER_SUBSPECTRUM * N_TIME_SAMPLES;
            for(int bors_i = 0; bors_i < n_bors; bors_i++) {
                size_t nhits = 0; 
                memcpy(&db_out->block[curblock_out].data,
                        &db_in->block[curblock_in].data,
                        N_DATA_BYTES_PER_BLOCK); 
                total_hits += nhits;
                clock_gettime(CLOCK_MONOTONIC, &stop);
                elapsed_gpu_ns += ELAPSED_NS(start, stop);
                gpu_block_count++;
            }
        }

        hashpipe_status_lock_safe(&st);
       	hputi4(st.buf, "NUMHITS", total_hits);
        hputr4(st.buf, "GPUMXERR", max_error);
        hputi4(st.buf, "GPUERCNT", error_count);
        hputi4(st.buf, "GPUMXECT", max_error_count);
        hashpipe_status_unlock_safe(&st);

        s6_output_databuf_set_filled(db_out, curblock_out);
        curblock_out = (curblock_out + 1) % db_out->header.n_block;

        hashpipe_databuf_set_free((hashpipe_databuf_t *)db_in, curblock_in);
        curblock_in = (curblock_in + 1) % db_in->header.n_block;

        /* Check for cancel */
        pthread_testcancel();
    }

	sem_unlink(gpu_sem_name);
    return NULL;
}

static hashpipe_thread_desc_t gpu_thread = {
    name: "s6_gpu_thread",
    skey: "GPUSTAT",
    init: NULL,
    run:  run,
    ibuf_desc: {s6_input_databuf_create},
    obuf_desc: {s6_output_databuf_create}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&gpu_thread);
}
