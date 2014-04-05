#include <stdio.h>

#define BUF_SZ 2048

static char input[BUF_SZ];

int main(int argc, char** argv) {


    for(;;) {
        fputs("> ", stdout);
        fgets(input, BUF_SZ, stdin);
        fputs(input, stdout);
    }


    return 0;
}
