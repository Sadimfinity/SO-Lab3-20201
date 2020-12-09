SRCS = saxpy-v0.c
TARG = saxpy-v0.out
OBJS = $(SRCS:.c=.o)

CC = gcc
OPTS = -Wall
LIBS = -lpthread

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    OPTS += -g -O0 -DDEBUG
else
    OPTS += -O3
endif

$(TARG): $(OBJS)
	$(CC) -o $(TARG) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(OPTS) -c $< -o $@ 

clean:
	rm -f $(OBJS) $(TARG) *~