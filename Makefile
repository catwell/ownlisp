CC= clang
CFLAGS= -std=c99 -Wall

prompt: prompt.c
	$(CC) $(CFLAGS) prompt.c -o prompt
