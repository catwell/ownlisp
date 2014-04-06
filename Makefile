CC= clang
CFLAGS= -std=c99 -Wall -g
LDFLAGS= -ledit -lm

SRCS= mpc.c ast.c builtin.c expr.c lenv.c lval.c prompt.c
OBJS= $(SRCS:.c=.o)

all: prompt

prompt: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o prompt

%.o: %.c mpc.h ownlisp.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f prompt *.o
