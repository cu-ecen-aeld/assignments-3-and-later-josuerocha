CC=gcc
DEPS = writer.c
OBJ = writer.o

ifdef CROSS_COMPILE
	CC := $(CROSS_COMPILE)$(CC)
endif

all: writer

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

writer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm *.o
	rm writer