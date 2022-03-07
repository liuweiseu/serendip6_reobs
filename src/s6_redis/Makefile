CC = gcc
TARGET = Redis_Demo
LINK = -lhiredis
INCLUDE = -I ./include

SRCS = ${wildcard ./source/*.cpp} main.cpp
OBJS = ${patsubst %.cpp, %.o, ${SRCS}}

${TARGET}: file $(OBJS)
	${CC} ${OBJS} -o $@ ${LINK}

%.o : %.c
	${CC} -c $< -o $@ ${INCLUDE} ${LINK}

file :
	@echo ${SRCS}
	@echo ${OBJS}

clean:
	@rm ${OBJS} 
	@rm ${TARGET}