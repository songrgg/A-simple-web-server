CC=gcc

TARGET	= server
OBJS		= server.o rio.o socket.o
CFLAGS	= -O2

.PHONY : everything clean all

everthing : $(TARGET)

clean :
	rm -f $(OBJS) $(TARGET)

all : clean everything

server : server.c rio.c socket.c
	$(CC) $(CFLAGS) -o $@ $< rio.c socket.c

