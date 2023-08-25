CROSS_COMPILE ?= mipsel-openipc-linux-musl-
CC = $(CROSS_COMPILE)gcc
CFLAGS = -fno-pie -Isrc/include
LDFLAGS = -s
LIBS = lib/libmp3lame.a lib/libimp.a lib/libalog.a

SRCS = src/player.c
OBJS = $(SRCS:.c=.o)

all: audioplayer

audioplayer: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) audioplayer
