
CC = gcc
CFLAGS = -g -I /usr/src/linux/include/ -I /usr/src/linux/arch/x86/include
LFLAGS = -g

PROJECT = main

OBJS = main.o

${PROJECT}: ${OBJS}
	${CC} ${LFLAGS} -o ${PROJECT} ${OBJS} ${LDFLAGS}

%.o: %.cpp
	${CC} -c ${CFLAGS} *.cpp

clean:
	rm -f core.* *.o *~ ${PROJECT}

