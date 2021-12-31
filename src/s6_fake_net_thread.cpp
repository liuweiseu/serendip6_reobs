/*
 * s6_fake_net_thread.c
 *
 * Routine to write fake data into shared memory blocks.  This allows the
 * processing pipelines to be tested without the network portion of SERENDIP6.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

#include "hashpipe.h"
#include "s6_gen_fake_data.h"
#include "s6_databuf.h"

static void *run(hashpipe_thread_args_t * args)
{
    s6_input_databuf_t *db  = (s6_input_databuf_t *)args->obuf;
    hashpipe_status_t *p_st = &(args->st);

    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;

    //s6_input_block_t fake_data_block;

    /* Main loop */
    int i, rv;
    uint64_t mcnt = 0;
    uint64_t num_coarse_chan = N_COARSE_CHAN;
    uint64_t *data;
    int block_idx = 0;
    int error_count = 0, max_error_count = 0;
    float error, max_error = 0.0;
    int gen_fake = 1;		// set this to 1 to turn on fake data generation
                            //  Once data have been generated of an input
                            //  ring buffer, no need to regenerate until
                            //  buffers are removed.

    hashpipe_status_lock_safe(&st);
    //hashpipe_status_lock_safe(p_st);
    hputi4(st.buf, "NUMCCHAN", N_COARSE_CHAN);
    hputi4(st.buf, "NUMFCHAN", N_FINE_CHAN);
    hputi4(st.buf, "NUMBBEAM", N_BYTES_PER_BEAM);
    hputi4(st.buf, "NUMBBLOC", sizeof(s6_input_block_t));
    hputi4(st.buf, "THRESHLD", POWER_THRESH);
    //hgeti4(st.buf, "GENFAKE", &gen_fake);
    hashpipe_status_unlock_safe(&st);
    //hashpipe_status_unlock_safe(p_st);

    time_t t, prior_t;
    prior_t = time(&prior_t);

    while (run_threads()) {

        hashpipe_status_lock_safe(&st);
        //hashpipe_status_lock_safe(p_st);
        hputi4(st.buf, "NETBKOUT", block_idx);
        hputs(st.buf, status_key, "waiting");
        hashpipe_status_unlock_safe(&st);
        //hashpipe_status_unlock_safe(p_st);
 
        t = time(&t);
        fprintf(stderr, "elapsed seconds for block %d : %ld\n", block_idx, t - prior_t);
        prior_t = t;
        // Wait for data
        struct timespec sleep_dur, rem_sleep_dur;
        sleep_dur.tv_sec = 1;
        sleep_dur.tv_nsec = 0;
        //fprintf(stderr, "fake net thread sleeping for %7.5f seconds\n", 
        //        sleep_dur.tv_sec + (double)sleep_dur.tv_nsec/1000000000.0);
        nanosleep(&sleep_dur, &rem_sleep_dur);
	
        /* Wait for new block to be free, then clear it
         * if necessary and fill its header with new values.
         */
        while ((rv=s6_input_databuf_wait_free(db, block_idx)) 
                != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                pthread_exit(NULL);
                break;
            }
        }

        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "receiving");
        hashpipe_status_unlock_safe(&st);
 
        // populate block header
        db->block[block_idx].header.mcnt = mcnt;
        db->block[block_idx].header.coarse_chan_id = 321;
        db->block[block_idx].header.num_coarse_chan = num_coarse_chan;
        memset(db->block[block_idx].header.missed_pkts, 0, sizeof(uint64_t) * N_BEAM_SLOTS);

        if(gen_fake) {
            //gen_fake = 0;
            // All blocks will contain same fake data.
            // TODO vary data by beam
#define SLOW_GEN
#ifdef SLOW_GEN 
            // Slow : gen fake data (with signals) for all beams, all blocks   
            fprintf(stderr, "slowly generating fake data (sine waves) to block 0 beam 0...\n");
            gen_fake_data(&(db->block[0].data[0]));
#else
			// Fast : quick and dirty data gen - saw tooth
            fprintf(stderr, "quickly generating fake data (saw tooth) to block 0 beam 0...\n");
			char * cdata;
			cdata = (char *)&(db->block[0].data[i]); 
			for(uint64_t i = 0; i <  N_TIME_SAMPLES; i++) cdata[i] = i % (N_TIME_SAMPLES / 512);
#endif
            fprintf(stderr, "done generating fake data to block 0 beam 0\n");
            for(int block_i = 1; block_i < N_INPUT_BLOCKS; block_i++) {
                fprintf(stderr, "copying fake data to block %d\n", block_i);
                memcpy((void *)&db->block[block_i].data[0], 
                    (void *)&db->block[0].data[0], 
                    N_DATA_BYTES_PER_BLOCK);
            }
        }

        hashpipe_status_lock_safe(&st);
        hputr4(st.buf, "NETMXERR", max_error);
        hputi4(st.buf, "NETERCNT", error_count);
        hputi4(st.buf, "NETMXECT", max_error_count);
        hashpipe_status_unlock_safe(&st);

        // Mark block as full
        s6_input_databuf_set_filled(db, block_idx);

        // Setup for next block
        block_idx = (block_idx + 1) % db->header.n_block;
        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "NET_BLKIN", block_idx);
        hashpipe_status_unlock_safe(&st);
        mcnt++;
        // uncomment the following to test dynamic setting of num_coarse_chan
        //num_coarse_chan--;

        /* Will exit if thread has been cancelled */
        pthread_testcancel();
    }

    // Thread success!
    return THREAD_OK;
}

static hashpipe_thread_desc_t fake_net_thread = {
    name: "s6_fake_net_thread",
    skey: "NETSTAT",
    init: NULL,
    run:  run,
    ibuf_desc: {NULL},
    obuf_desc: {s6_input_databuf_create}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&fake_net_thread);
}
