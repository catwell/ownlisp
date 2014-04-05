#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <histedit.h>

#include "mpc.h"

int main(int argc, char** argv) {
    char* input;
    mpc_result_t mpc_result;

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(
        MPC_LANG_DEFAULT,
        "number   :  /-?[0-9]+/ ;"
        "operator :  '+' | '-' | '*' | '/' ;"
        "expr     :  <number> | '(' <operator> <expr>+ ')' ;"
        "lispy    :  /^/ <operator> <expr>+ /$/ ;",
        Number, Operator, Expr, Lispy
    );

    for(;;) {

        input = readline("> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, Lispy, &mpc_result)) {
            mpc_ast_print(mpc_result.output);
            mpc_ast_delete(mpc_result.output);
        }
        else {
            mpc_err_print(mpc_result.error);
            mpc_err_delete(mpc_result.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}
