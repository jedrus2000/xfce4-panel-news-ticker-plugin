src =	test.c

obj =	$(src:.c=.o)

CC =	gcc

CFLAGS = -O2 -Wall -Wextra `pkg-config --cflags glib-2.0`

LIBS =	--mms-bitfields `pkg-config --libs glib-2.0`

all:	test

$(obj): $(src) Makefile
	$(CC) $(CFLAGS) -c $(src)

test: $(obj)
	$(CC) -o test $(obj) $(LIBS)
