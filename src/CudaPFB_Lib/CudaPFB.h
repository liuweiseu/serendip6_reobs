/* fast_gpu.h */
#ifndef _CUDAPFB_H
#define _CUDAPFB_H

#define TAPS 4

typedef struct FFT_RES {
    float re;
    float im;
}FFT_RES;

#define DIN_TYPE    char
#define DOUT_TYPE   float //struct FFT_RES

#define CHANNELS    65536 //65536
#define SPECTRA     4096
#define SAMPLES     CHANNELS * (SPECTRA + TAPS - 1)

#define START_BIN   4608 //0
#define STOP_BIN    4863 //255
#define CH_PER_SPEC (STOP_BIN - START_BIN + 1)
#define OUTPUT_LEN  SPECTRA * CH_PER_SPEC * 2 // multiplying 2 is for re and im parts

void GPU_GetDevInfo();
int GPU_SetDevice(int gpu_dev);
int Host_MallocBuffer(DIN_TYPE **buf_in, DOUT_TYPE **buf_out);
void GPU_MallocBuffer();
int GPU_CreateFFTPlan();
void GPU_MoveWeightsFromHost(float *weights);
void GPU_MoveDataFromHost(DIN_TYPE *din);
void GPU_MoveDataToHost(FFT_RES *dout);
int GPU_DoPFB();
void GPU_DestroyPlan();
void Host_FreeBuffer(DIN_TYPE *buf_in, DOUT_TYPE *buf_out);
void GPU_FreeBuffer();
#endif
