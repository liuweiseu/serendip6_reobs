/*******************************************************************************
 * Copyright (c) 2020-2021, National Research Foundation (SARAO)
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy
 * of the License at
 *
 *   https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/


#ifndef PORT_MAKO
#define PORT_MAKO

#ifdef __OPENCL_VERSION__

#define DEVICE_FN
#define KERNEL __kernel
#define GLOBAL_DECL __global
#define GLOBAL __global
#define LOCAL_DECL __local
#define LOCAL __local
#define BARRIER() barrier(CLK_LOCAL_MEM_FENCE)
#define RESTRICT restrict
#define REQD_WORK_GROUP_SIZE(x, y, z) __attribute__((reqd_work_group_size(x, y, z)))
#define SHUFFLE_AVAILABLE 0


DEVICE_FN char2 make_char2(char x, char y)
{
    return (char2) (x, y);
}

DEVICE_FN char4 make_char4(char x, char y, char z, char w)
{
    return (char4) (x, y, z, w);
}


DEVICE_FN uchar2 make_uchar2(uchar x, uchar y)
{
    return (uchar2) (x, y);
}

DEVICE_FN uchar4 make_uchar4(uchar x, uchar y, uchar z, uchar w)
{
    return (uchar4) (x, y, z, w);
}


DEVICE_FN short2 make_short2(short x, short y)
{
    return (short2) (x, y);
}

DEVICE_FN short4 make_short4(short x, short y, short z, short w)
{
    return (short4) (x, y, z, w);
}


DEVICE_FN ushort2 make_ushort2(ushort x, ushort y)
{
    return (ushort2) (x, y);
}

DEVICE_FN ushort4 make_ushort4(ushort x, ushort y, ushort z, ushort w)
{
    return (ushort4) (x, y, z, w);
}


DEVICE_FN int2 make_int2(int x, int y)
{
    return (int2) (x, y);
}

DEVICE_FN int4 make_int4(int x, int y, int z, int w)
{
    return (int4) (x, y, z, w);
}


DEVICE_FN uint2 make_uint2(uint x, uint y)
{
    return (uint2) (x, y);
}

DEVICE_FN uint4 make_uint4(uint x, uint y, uint z, uint w)
{
    return (uint4) (x, y, z, w);
}


DEVICE_FN long2 make_long2(long x, long y)
{
    return (long2) (x, y);
}

DEVICE_FN long4 make_long4(long x, long y, long z, long w)
{
    return (long4) (x, y, z, w);
}


DEVICE_FN ulong2 make_ulong2(ulong x, ulong y)
{
    return (ulong2) (x, y);
}

DEVICE_FN ulong4 make_ulong4(ulong x, ulong y, ulong z, ulong w)
{
    return (ulong4) (x, y, z, w);
}


DEVICE_FN float2 make_float2(float x, float y)
{
    return (float2) (x, y);
}

DEVICE_FN float4 make_float4(float x, float y, float z, float w)
{
    return (float4) (x, y, z, w);
}


#else

#include <math.h>
#include <float.h>
#include <stdio.h>

/* System headers may provide some of these, but it's OS dependent. It's
 * legal to repeat typedefs, so make sure that they're all available for
 * consistency with OpenCL.
 */
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#define DEVICE_FN __device__
#define KERNEL __global__
#define GLOBAL_DECL __global__
#define GLOBAL
#define LOCAL_DECL __shared__
#define LOCAL
#define BARRIER() __syncthreads()
#define RESTRICT __restrict
#define REQD_WORK_GROUP_SIZE(x, y, z) __launch_bounds__((x) * (y) * (z))

#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 300
# define SHUFFLE_AVAILABLE 1
#else
# define SHUFFLE_AVAILABLE 0
#endif

__device__ static inline unsigned int get_local_id(int dim)
{
    return dim == 0 ? threadIdx.x : dim == 1 ? threadIdx.y : threadIdx.z;
}

__device__ static inline unsigned int get_group_id(int dim)
{
    return dim == 0 ? blockIdx.x : dim == 1 ? blockIdx.y : blockIdx.z;
}

__device__ static inline unsigned int get_local_size(int dim)
{
    return dim == 0 ? blockDim.x : dim == 1 ? blockDim.y : blockDim.z;
}

__device__ static inline unsigned int get_global_id(int dim)
{
    return get_group_id(dim) * get_local_size(dim) + get_local_id(dim);
}

__device__ static inline unsigned int get_num_groups(int dim)
{
    return dim == 0 ? gridDim.x : dim == 1 ? gridDim.y : gridDim.z;
}

__device__ static inline float as_float(unsigned int x)
{
    return __int_as_float(x);
}

__device__ static inline int as_int(float x)
{
    return __float_as_int(x);
}

#endif /* CUDA */
#endif /* PORT_MAKO */


#define WGS 256 //128
#define TAPS 8

/* Each work-item is responsible for a run of input values with stride `step`.
 *
 * This approach becomes very register-heavy as the number of taps increases.
 * A better approach may be to have the work group cooperatively load a
 * rectangle of data into local memory, transpose, and work from there. While
 * local memory is smaller than the register file, multiple threads will read
 * the same value.
 */
KERNEL REQD_WORK_GROUP_SIZE(WGS, 1, 1) void pfb_fir(
    GLOBAL float * RESTRICT out,          // Output memory
    const GLOBAL char * RESTRICT in,     // Input data (digitiser samples)
    const GLOBAL float * RESTRICT weights,// Weights for the PFB-FIR filter.
    int n,                                // Size of the `out` array, to avoid going out-of-bounds.
    int step,                             // Number of input samples needed for each spectrum, i.e. 2*channels.
    int stepy,                            // Size of data that will be worked on by a single thread-block.
    int in_offset,                        // Number of samples to skip from the start of *in.
    int out_offset           // Number of samples to skip from the start of *out. Must be a multiple of `step` to make sense.
)
{
    int pos = (blockIdx.x + blockIdx.y * gridDim.x) * WGS + threadIdx.x; 
    float tmp = 0; 
#pragma unroll 8
    for (int i = 0; i < TAPS; i++)  // We'll be at our most memory-bandwidth-efficient if rows >> TAPS. Launching ~256K threads should ensure this.
    {
        tmp += in[pos + i * step] * weights[pos%step + i * step];
    }
    out[pos] = tmp;
}

