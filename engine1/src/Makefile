OBJ_DIR = ../bld

SRC = main.c junqi.c comm.c
  
OBJ = ${patsubst %.c, ${OBJ_DIR}/%.o, ${SRC}}

INC = ${wildcard  *.h}

LIBS = 

TARGET=../engine1
CC=gcc
CCFLAGS=-g -Wall  

all:${TARGET}

${TARGET}: ${OBJ} 
	${CC} $^ -o $@ ${LIBS} 
	@echo "Compile done."
    

${OBJ_DIR}/%.o:%.c ${INC}
	@echo "Compiling $< ==> $@"
	${CC} ${CCFLAGS} -c $< -o $@

    
clean:
	rm -f ${OBJ_DIR}/*.o
	rm -f ${TARGET}