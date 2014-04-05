CC= clang
CFLAGS= -std=c99 -Wall
LDFLAGS= -ledit

prompt: prompt.c
	$(CC) $(CFLAGS) $(LDFLAGS) prompt.c -o prompt

clean:
	rm -f prompt
