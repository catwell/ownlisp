#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <editline/readline.h>
#include <histedit.h>

#include "mpc.h"

long eval(mpc_ast_t *t);

long eval_plus(mpc_ast_t** children, int num) {
    assert(num > 0);

    int r = 0;

    while(num) {
        r = r + eval(*children);
        children++, num--;
    }

    return r;
}

long eval_minus(mpc_ast_t** children, int num) {
    assert(num > 0);

    int r;

    if (num == 1) {
        r = 0;
    }
    else {
        r = eval(*children);
        children++, num--;
    }

    while(num) {
        r = r - eval(*children);
        children++, num--;
    }

    return r;
}

long eval_mul(mpc_ast_t** children, int num) {
    assert(num > 0);

    int r = 1;

    while(num) {
        r = r * eval(*children);
        children++, num--;
    }

    return r;
}

long eval_div(mpc_ast_t** children, int num) {
    assert(num == 2);

    return eval(children[0]) / eval(children[1]);
}

long eval_mod(mpc_ast_t** children, int num) {
    assert(num == 2);

    return eval(children[0]) % eval(children[1]);
}

long eval_min(mpc_ast_t** children, int num) {
    assert(num > 0);

    int cur;
    int r = eval(*children);
    children++, num--;

    while(num) {
        cur = eval(*children);
        if (cur < r) r = cur;
        children++, num--;
    }

    return r;
}

long eval_max(mpc_ast_t** children, int num) {
    assert(num > 0);

    int cur;
    int r = eval(*children);
    children++, num--;

    while(num) {
        cur = eval(*children);
        if (cur > r) r = cur;
        children++, num--;
    }

    return r;
}

long eval(mpc_ast_t *t) {

    char *op;
    mpc_ast_t** children;
    int num;


    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }

    op = t->children[1]->contents;
    children = t->children + 2;
    num = t->children_num - 3;

    if (!strcmp(op, "+")) return eval_plus(children, num);
    if (!strcmp(op, "-")) return eval_minus(children, num);
    if (!strcmp(op, "*")) return eval_mul(children, num);
    if (!strcmp(op, "/")) return eval_div(children, num);
    if (!strcmp(op, "\%")) return eval_mod(children, num);
    if (!strcmp(op, "min")) return eval_min(children, num);
    if (!strcmp(op, "max")) return eval_max(children, num);

    assert(0);
}

int main(int argc, char** argv) {
    char* input;
    mpc_result_t mpc_result;
    long result;

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(
        MPC_LANG_DEFAULT,
        "number   :  /-?[0-9]+/ ;"
        "operator :  '+' | '-' | '*' | '/' | '\%' | \"min\" | \"max\" ;"
        "expr     :  <number> | '(' <operator> <expr>+ ')' ;"
        "lispy    :  /^/ <operator> <expr>+ /$/ ;",
        Number, Operator, Expr, Lispy
    );

    for(;;) {

        input = readline("> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, Lispy, &mpc_result)) {
            result = eval(mpc_result.output);
            printf("%ld\n", result);
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
