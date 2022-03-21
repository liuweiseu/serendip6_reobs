CC          = gcc
# The user should specify, on the 'make' command line :
# S6_LOCATION="-D SOURCE_DIBAS"
# or
# S6_LOCATION="-D SOURCE_S6"
S6_LOCATION = "-D SOURCE_FAST"
#S6_LIB_CCFLAGS     = -g -O3 -fPIC -shared -lstdc++ -mavx -msse4
S6_FAST_LIB_CCFLAGS     = -g -O3 -fPIC -shared -lstdc++ \
		     -I. -I$(CUDA_DIR)/include -I$(CUDA_DIR)/targets/x86_64-linux/include -I/usr/local/include -I/usr/include/cfitsio -I/usr/local/cuda/include -I. \
		     -L. -L/usr/local/lib -L/usr/lib64 -L/usr/local/lib/hiredis -L./CudaPFB_Lib\
		     -lhashpipe -lrt -lm -lcudapfb -std=c++14\

S6_FAST_LIB_TARGET   = 	serendip6.so
S6_FAST_LIB_SOURCES  = 	s6_databuf.cpp \
		  				s6_pktsock_thread.cpp \
		  				s6_gen_fake_data.cpp \
		  				s6_fake_net_thread.cpp \
		  				s6_gpu_thread.cpp \
		  				s6_output_thread.cpp \
						s6_write_raw_data/source/s6_write_raw_data.cpp \


S6_FAST_LIB_INCLUDES = s6_databuf.h \
		  
SUB_DIR = CudaPFB_Lib

all_fast: ${SUB_DIR} $(S6_FAST_LIB_TARGET) 

$(S6_FAST_LIB_TARGET):$(S6_FAST_LIB_SOURCES) $(S6_FAST_LIB_INCLUDES) ;
	$(CC)   -o $(S6_FAST_LIB_TARGET) $(S6_FAST_LIB_SOURCES) $(S6_FAST_LIB_CCFLAGS) $(S6_LOCATION)

${SUB_DIR}: ECHO
	make -C $@

ECHO:
	@echo Going to compile .so in ${SUB_DIR}...

tags:
	ctags -R .

clean:
	rm -f $(S6_FAST_LIB_TARGET)  tags
	make -C ${SUB_DIR} clean

prefix=/usr/local
LIBDIR=$(prefix)/lib


install-lib:  $(S6_FAST_LIB_TARGET)
	mkdir -p "$(DESTDIR)$(LIBDIR)"
	install -p $^ "$(DESTDIR)$(LIBDIR)"
	make -C ${SUB_DIR} install

install: install-lib 

.PHONY: all tags clean install install-lib

