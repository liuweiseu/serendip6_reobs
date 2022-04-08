NVCC	= nvcc

TARGET  = libcudapfb.so

INC 	= pfb_fir.cuh \
		  CudaPFB.h
SRC		= CudaPFB.cu

NVCCFLAG  = -Xcompiler -fPIC --shared \
			-I/usr/local/cuda/include \
		  	-L/usr/local/cuda/lib64   \
		  	-lcufft	-arch=sm_61				

${TARGET}: ${SRC} ${INC} 
	${NVCC} -o $@ ${SRC} ${NVCCFLAG}


.PHONY: clean install
install:
	install -p ${TARGET} "/usr/local/lib" 
clean:
	rm ${TARGET} 