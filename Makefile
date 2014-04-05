CC= clang
CFLAGS= -std=c99 -Wall
LDFLAGS= -ledit -lm

prompt: prompt.c
	$(CC) $(CFLAGS) $(LDFLAGS) prompt.c mpc.c -o prompt

clean:
	rm -f prompt
