#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <histedit.h>

int main(int argc, char** argv) {
    char* input;

    for(;;) {

        input = readline("> ");
        add_history(input);
        puts(input);
        free(input);
    }

    return 0;
}
