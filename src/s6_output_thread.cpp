/*
 * s6_output_thread.c
 */

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

/*
#include <cuda.h>
#include <cufft.h>
*/
#include <sched.h>

#include "hashpipe.h"
#include "s6_databuf.h"
#include "s6_write_raw_data/include/s6_write_raw_data.h"

#define SET_BIT(val, bitIndex) val |= (1 << bitIndex)
#define CLEAR_BIT(val, bitIndex) val &= ~(1 << bitIndex)
#define BIT_IS_SET(val, bitIndex) (val & (1 << bitIndex))

static int file_state = 0;

static int write_to_file(s6_output_databuf_t *db, int block_idx, int newfile, char *compute_node, int beam, int pol)
{   
    char filename[256] = "\0";
    if(newfile==1 && file_state == 0)
    {
        create_rawdata_filename(compute_node,FREQ_RANGE, beam, pol, 
                                db->block[block_idx].header.time_sec, 
                                db->block[block_idx].header.time_nsec, filename);
        open_rawdata_file(filename);
        fprintf(stderr, "Filename: %s\r\n", filename);
        file_state = 1;
    }
    else if(newfile == 0 && file_state == 1)
    {
        close_rawdata_file();
        file_state = 0;
    }
    else if(newfile == 1 && file_state == 1)
    {
        write_rawdata((char*)&db->block[block_idx].data, N_DATA_BYTES_PER_BLOCK);
        //fwrite(&db->block[block_idx].data,N_DATA_BYTES_PER_BLOCK,1,fp);
    }
    else if(newfile ==0 && file_state == 0)
    {
        // nothing to do
    }

    return 0;
}
static int init(hashpipe_thread_args_t *args)
{
    hashpipe_status_t st = args->st;

    hashpipe_status_lock_safe(&st);
    //hputr4(st.buf, "CGOMXERR", 0.0);
    //hputi4(st.buf, "CGOERCNT", 0);
    //hputi4(st.buf, "CGOMXECT", 0);
    hashpipe_status_unlock_safe(&st);

    // Success!
    return 0;
}

static void *run(hashpipe_thread_args_t * args)
{
    // Local aliases to shorten access to args fields
    // Our input buffer happens to be a s6_ouput_databuf
    s6_output_databuf_t *db = (s6_output_databuf_t *)args->ibuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;

// TODO all of the SOURCE_FAST sections are copies of SOURCE_DIBAS sections as place holders.
    char *prior_receiver = (char *)malloc(32);
 

    int run_always, prior_run_always=0;                 // 1 = run even if no receiver

    int idle=0;                                         // 1 = idle output, 0 = good to go
    uint32_t idle_flag=0;                               // bit field for data driven idle conditions    
    int testmode=1;                                     // modified by Wei on 12/30/2021. Let's make it work on testmode
                                                        // 1 = write output file regardless of other
                                                        //   flags and do not attempt to obtain observatory
                                                        //   status data (in fact, write zeros to the obs
                                                        //   status structure).  0 = operate normally and
                                                        //   respect other flags.

                                                        // data driven idle bit indexes
    int idle_redis_error = 1; 
    int idle_zero_IFV1BW = 2; 
    size_t num_coarse_chan = 0;
    
    int newfile = 0;                                    // 1 = create a new file; 0 = keep the current state
    char compute_node[4] = "\0";                        // compute node  from hashpipe buffer
    int beam, pol;                                      // get beam and pol from hashpipe buffer
    
    extern const char *receiver[];

    int i, rv=0, debug=20;
    int block_idx=0;
    int error_count, max_error_count = 0;
    float error, max_error = 0.0;
	time_t time_start, runsecs;

    char current_filename[200] = "\0";  // init as a null string


	time_start = time(NULL);

    /* Main loop */
    while (run_threads()) {

		runsecs = time(NULL) - time_start;
        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "OUTBLKIN", block_idx);
        hputs(st.buf, status_key, "waiting");
        hgeti4(st.buf, "RUNALWYS", &run_always);
        hputi4(st.buf, "RUNSECS", runsecs);
        hashpipe_status_unlock_safe(&st);

       // get new data
       while ((rv=s6_output_databuf_wait_filled(db, block_idx))
                != HASHPIPE_OK) {
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

        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "processing");
        hashpipe_status_unlock_safe(&st);
        // TODO check mcnt

		// time stamp for this block of hits

        hgeti4(st.buf, "IDLE", &idle);
        hgeti4(st.buf, "TESTMODE", &testmode);

    // Start idle checking
    // generic redis error check. 

        hputi4(st.buf, "IDLE", idle);   // finally, make our idle condition live
    // End idle checking

        hashpipe_status_lock_safe(&st);

        hashpipe_status_unlock_safe(&st);

        hgeti4(st.buf, "NEWFILE", &newfile);
        hgets(st.buf,"COMPUTENODE",4,compute_node);
        hgeti4(st.buf,"FASTBEAM",&beam);
        hgeti4(st.buf,"FASTPOL", &pol);
        if(testmode || run_always) {
            rv = write_to_file(db,block_idx, newfile, compute_node, beam, pol);
            if(rv) {
                hashpipe_error(__FUNCTION__, "error error returned from write_to_file()");
                pthread_exit(NULL);
            }
        }

        hashpipe_status_lock_safe(&st);
        // TODO error counts not yet implemented
        hputr4(st.buf, "OUTMXERR", max_error);
        hputi4(st.buf, "OUTERCNT", error_count);
        hputi4(st.buf, "OUTMXECT", max_error_count);
        // put a few selected coarse channel powers to the status buffer
        hashpipe_status_unlock_safe(&st);

        // Mark block as free
        memset((void *)&db->block[block_idx], 0, sizeof(s6_output_block_t));   
        s6_output_databuf_set_free(db, block_idx);

        // Setup for next block
        block_idx = (block_idx + 1) % db->header.n_block;    

        /* Will exit if thread has been cancelled */
        pthread_testcancel();
    }

    return NULL;
}

static hashpipe_thread_desc_t output_thread = {
    name: "s6_output_thread",
    skey: "OUTSTAT",
    init: init,
    run:  run,
    ibuf_desc: {s6_output_databuf_create},
    obuf_desc: {NULL}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&output_thread);
}
