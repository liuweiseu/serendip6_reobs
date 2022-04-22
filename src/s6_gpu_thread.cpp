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
#include "CudaPFB_Lib/CudaPFB.h"

#define ELAPSED_NS(start,stop) \
  (((int64_t)stop.tv_sec-start.tv_sec)*1000*1000*1000+(stop.tv_nsec-start.tv_nsec))

#define SIN_WAVE 1
// cal rms
static void cal_rms(FFT_RES *d, FFT_RES *rms, FFT_RES *average)
{
    int N = OUTPUT_LEN/2;
    float sum_p_re = 0, sum_p_im = 0;
    float sum_re = 0, sum_im = 0;
    for(int i = 0; i < N; i++)
    {
        sum_p_re += d[i].re * d[i].re;
        sum_p_im += d[i].im * d[i].im;
        sum_re += d[i].re;
        sum_im += d[i].im;
    }
    rms->re = sqrt(sum_p_re / N);
    rms->im = sqrt(sum_p_im / N);
    average->re = sum_re / N;
    average->im = sum_im / N;
}

static void adj_gain_static(FFT_RES *din, FFT_RES *rms, FFT_RES *average, float gain, char *dout)
{
    int N = OUTPUT_LEN/2;
    int tmp = 0;
    
    float sum_p_re = 0, sum_p_im = 0;
    float sum_re = 0, sum_im = 0;

    for(int i = 0; i < N; i ++)
    {
        tmp = (int)(din[i].re * gain);
        dout[2*i] = (tmp > 127)?127:((tmp < -127)?-127:tmp);
        tmp = (int)(din[i].im * gain);
        dout[2*i+1] = (tmp > 127)?127:((tmp < -127)?-127:tmp);

        sum_p_re += dout[2*i] * dout[2*i];
        sum_p_im += dout[2*i+1] * dout[2*i+1];

        sum_re += dout[2*i];
        sum_im += dout[2*i+1];
    } 

    rms->re = sqrt(sum_p_re / N);
    rms->im = sqrt(sum_p_im / N);
    average->re = sum_re / N;
    average->im = sum_im / N;
}

static int init(hashpipe_thread_args_t *args)
{
    /*
    hashpipe_status_t st = args->st;
    hashpipe_status_lock_safe(&st);
    hashpipe_status_unlock_safe(&st);
    */
    
    // Success!
    return 0;
}

static void *run(hashpipe_thread_args_t * args)
{
    
    // Local aliases to shorten access to args fields
    s6_input_databuf_t *db_in = (s6_input_databuf_t *)args->ibuf;
    s6_output_databuf_t *db_out = (s6_output_databuf_t *)args->obuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;

    int rv, status;
    int curblock_in=0;
    int curblock_out=0;

    FFT_RES rms_before;                                        // rms of re and im
    FFT_RES average_before;                                    // average of re and im
    FFT_RES rms_after;
    FFT_RES average_after;

    FFT_RES *data_p = (FFT_RES*) malloc(N_DATA_BYTES_PER_OUT_BLOCK);
    float gain = 0;
     /*
    * The following is about GPU init
    */
    // Check gpu status
    //int status;
    //GPU_GetDevInfo();

    // get gpu dev from hashpipe buffer
    int gpudev = 0;
    hgeti4(st.buf,"GPUDEV", &gpudev);
    status = GPU_SetDevice(gpudev);
    
    /*
    if(status < 0)
        printf("No device will handle overlaps.\r\n");
    else   
        printf("overlaps are supported on the device.\r\n");
    */

    // Malloc buffer on GPU
    GPU_MallocBuffer();

    // Preparing weights for PFB FIR
    float *weights;
    weights = (float*) malloc(TAPS*CHANNELS*sizeof(float));
    //printf("preparing for weights...\r\n");
    char wfile[100]={0};
    hgets(st.buf,"WEIGHTS",100,wfile);
    printf("%s\r\n",wfile);
    FILE *fp_weights;
    fp_weights = fopen(wfile,"r");
    if(fp_weights==NULL)
        printf("file can't be opened.\r\n");
    size_t r = 0;
    r = fread((float*)weights,sizeof(float),TAPS*CHANNELS,fp_weights);
    fclose(fp_weights);

    //create tap data buffer
    char *d_tap = (char *)malloc((TAPS-1)*CHANNELS*sizeof(char));
    memset(d_tap, 0 , (TAPS-1)*CHANNELS*sizeof(char));

    GPU_MoveWeightsFromHost(weights);
    
      
    // create cufft plan
    status = GPU_CreateFFTPlan();
    if(status == -1)
    {
        printf("The cuFFT plan can't be created!\r\n");
        return 0;
    }
    else
        printf("The cuFFT plan is created successfully!\r\n");

    struct timespec start, stop;
    uint64_t elapsed_gpu_ns  = 0;

	/*
    char gpu_sem_name[256];
	sem_t * gpu_sem;
	sprintf(gpu_sem_name, "serendip6_gpu_sem_device_%d", gpu_dev);
	gpu_sem = sem_open(gpu_sem_name, O_CREAT, S_IRWXU, 1);
	*/
    
    while (run_threads()) {

        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "GPUBLKIN", curblock_in);
        hputs(st.buf, status_key, "waiting");
        hputi4(st.buf, "GPUBKOUT", curblock_out);
        hashpipe_status_unlock_safe(&st);
        hgetr4(st.buf,"GAIN",&gain);
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

        // Note processing status
        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "processing gpu");
        hashpipe_status_unlock_safe(&st);

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

        GPU_MoveDataFromHost((char*)db_in->block[curblock_in].data, d_tap);
        status = GPU_DoPFB();
        if(status == -1)
        {   
            fprintf(stderr, "PFB failed!\r\n");
            return NULL;
        }
        /*
        else
        {
            fprintf(stderr, "PFB Success!\r\n");
        } 
        */
        //GPU_MoveDataToHost((FFT_RES*)(db_out->block[curblock_out].data));
        GPU_MoveDataToHost(data_p);
        
        cal_rms(data_p, &rms_before, &average_before);
        //adj_gain(data_p, &rms, &average, &max, db_out->block[curblock_out].data);
        adj_gain_static(data_p, &rms_after, &average_after, gain, db_out->block[curblock_out].data);

        hashpipe_status_lock_safe(&st);
        hputr4(st.buf, "RMS_RE_BEF", rms_before.re);
        hputr4(st.buf, "RMS_IM_BEF", rms_before.im);
        hputr4(st.buf, "AVE_RE_BEF", average_before.re);
        hputr4(st.buf, "AVE_IM_BEF", average_before.im);
        hputr4(st.buf, "RMS_RE_AFT", rms_after.re);
        hputr4(st.buf, "RMS_IM_AFT", rms_after.im);
        hputr4(st.buf, "AVE_RE_AFT", average_after.re);
        hputr4(st.buf, "AVE_IM_AFT", average_after.im);
        hashpipe_status_unlock_safe(&st);

        s6_output_databuf_set_filled(db_out, curblock_out);
        curblock_out = (curblock_out + 1) % db_out->header.n_block;

        memcpy(d_tap, (db_in->block[curblock_in].data)+(SPECTRA-TAPS+1)*CHANNELS, (TAPS-1)*CHANNELS);
        hashpipe_databuf_set_free((hashpipe_databuf_t *)db_in, curblock_in);
        curblock_in = (curblock_in + 1) % db_in->header.n_block;

        /* Check for cancel */
        pthread_testcancel();
    }
    GPU_DestroyPlan();
    GPU_FreeBuffer();
    free(data_p);
    free(d_tap);
	//sem_unlink(gpu_sem_name);
    return NULL;
}

static hashpipe_thread_desc_t gpu_thread = {
    name: "s6_gpu_thread",
    skey: "GPUSTAT",
    init: init,
    run:  run,
    ibuf_desc: {s6_input_databuf_create},
    obuf_desc: {s6_output_databuf_create}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&gpu_thread);
}
