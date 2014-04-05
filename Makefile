CC= clang
CFLAGS= -std=c99 -Wall -g
LDFLAGS= -ledit -lm

prompt: prompt.c
	$(CC) $(CFLAGS) $(LDFLAGS) prompt.c mpc.c -o prompt

clean:
	rm -f prompt
