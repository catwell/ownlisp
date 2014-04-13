#include <editline/readline.h>
#include <histedit.h>

#include "ownlisp.h"

int main(int argc, char** argv) {
    char* input;
    mpc_result_t mpc_result;
    lval *result;

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Qexpr = mpc_new("qexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(
        MPC_LANG_DEFAULT,
        "number   :  /-?[0-9]+/ ;"
        "symbol   :  /[a-zA-Z0-9_+\\-*\\/\%\\\\=<>!&|]+/ ;"
        "sexpr    :  '(' <expr>* ')' ;"
        "qexpr    :  '{' <expr>* '}' ;"
        "expr     :  <number> | <symbol> | <sexpr> | <qexpr> ;"
        "lispy    :  /^/ <expr>* /$/ ;",
        Number, Symbol, Sexpr, Qexpr, Expr, Lispy
    );

    lenv *env = lenv_new();
    register_builtins(env);

    for(;;) {

        input = readline("> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, Lispy, &mpc_result)) {
            result = ast_read(mpc_result.output);
            if(DEBUG) lval_println(result);
            result = lval_eval(result, env);
            lval_println(result);
            lval_del(result);
            mpc_ast_delete(mpc_result.output);
        }
        else {
            mpc_err_print(mpc_result.error);
            mpc_err_delete(mpc_result.error);
        }

        free(input);
    }

    lenv_del(env);
    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}
