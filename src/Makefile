NVCC        = nvcc
NVCCFLAGS   = -O3 --compiler-options '-fPIC' --shared -Xcompiler -Wall -arch=sm_35 -lrt -lcudart -lcufft -lineinfo -std=c++11
CC          = gcc
# The user should specify, on the 'make' command line :
# S6_LOCATION="-D SOURCE_DIBAS"
# or
# S6_LOCATION="-D SOURCE_S6"
S6_LOCATION = 
#S6_LIB_CCFLAGS     = -g -O3 -fPIC -shared -lstdc++ -mavx -msse4
#S6_FAST_LIB_CCFLAGS modified by Wei , original flgas have -lS6GPU inside
S6_FAST_LIB_CCFLAGS     = -g -O3 -fPIC -shared -lstdc++ \
		     -I. -I$(CUDA_DIR)/include -I$(CUDA_DIR)/targets/x86_64-linux/include -I/usr/local/include -I/usr/include/cfitsio -I/usr/local/cuda-11.2/include -I. \
		     -L. -L/usr/local/lib -L/usr/lib64 -L/usr/local/lib/hiredis \
		     -lhashpipe -lrt -lhiredis -lcfitsio -lm -std=c++11 \
			 -lfastgpu

S6_FAST_LIB_TARGET   = serendip6.so
S6_FAST_LIB_SOURCES  = s6_databuf.cpp \
		  s6_pktsock_thread.cpp \
		  s6_gen_fake_data.cpp \
		  s6_fake_net_thread.cpp \
		  s6_gpu_thread.cpp \
		  s6_output_thread.cpp 

S6_FAST_LIB_INCLUDES = s6_databuf.h \
		  
S6_SCRIPT_TARGET = s6_init.sh

all_fast: $(S6_FAST_LIB_TARGET)

$(S6_FAST_LIB_TARGET):         $(S6_FAST_LIB_SOURCES) $(S6_FAST_LIB_INCLUDES) ;                 $(CC)   -o $(S6_FAST_LIB_TARGET) $(S6_FAST_LIB_SOURCES) $(S6_FAST_LIB_CCFLAGS) $(S6_LOCATION)

tags:
	ctags -R .

clean: ; rm -f $(S6_FAST_LIB_TARGET)  tags

prefix=/usr/local
LIBDIR=$(prefix)/lib


install-lib:  $(S6_FAST_LIB_TARGET)
	mkdir -p "$(DESTDIR)$(LIBDIR)"
	install -p $^ "$(DESTDIR)$(LIBDIR)"

install: install-lib 

.PHONY: all tags clean install install-lib
# vi: set ts=8 noet :
