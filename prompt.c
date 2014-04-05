#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <editline/readline.h>
#include <histedit.h>

#include "mpc.h"

typedef struct {
    int type;
    union {
        long num;
        int err;
    };
} lval;

/* value types */
enum {
    LVAL_ERR,
    LVAL_NUM
};

/* error types */
enum {
    LERR_BAD_OP,
    LERR_DIV_ZERO,
    LERR_BAD_NUM,
    LERR_BAD_ARITY
};

lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {
    switch (v.type) {
        case LVAL_ERR:
            switch (v.err) {
                case LERR_BAD_OP:
                    puts("ERROR: invalid operator");
                break;
                case LERR_DIV_ZERO:
                    puts("ERROR: division by 0");
                break;
                case LERR_BAD_NUM:
                    puts("ERROR: bad number");
                break;
                case LERR_BAD_ARITY:
                    puts("ERROR: bad arity");
                break;
                default:
                    assert(0);
            }
        break;
        case LVAL_NUM:
            printf("%ld", v.num);
        break;
        default:
            assert(0);
    }
}

void lval_println(lval v) {
    lval_print(v);
    putchar('\n');
}

lval eval(mpc_ast_t *t);

lval eval_num(mpc_ast_t *t) {
    lval r = eval(t);
    if (r.type == LVAL_ERR) return r;
    if (r.type != LVAL_NUM) return lval_err(LERR_BAD_NUM);
    return r;
}

lval eval_plus(mpc_ast_t** children, int n) {
    if(n <= 0) return lval_err(LERR_BAD_ARITY);

    lval c;
    lval r = lval_num(0);

    while(n) {
        c = eval_num(*children);
        if (c.type == LVAL_ERR) return c;
        r.num += c.num;
        children++, n--;
    }

    return r;
}

lval eval_minus(mpc_ast_t** children, int n) {
    if(n <= 0) return lval_err(LERR_BAD_ARITY);

    lval c;
    lval r = lval_num(0);

    if (n > 1) {
        c = eval_num(*children);
        if (c.type == LVAL_ERR) return c;
        r.num = c.num;
        children++, n--;
    }

    while(n) {
        c = eval_num(*children);
        if (c.type == LVAL_ERR) return c;
        r.num -= c.num;
        children++, n--;
    }

    return r;
}

lval eval_mul(mpc_ast_t** children, int n) {
    if(n <= 0) return lval_err(LERR_BAD_ARITY);

    lval c;
    lval r = lval_num(1);

    while(n) {
        c = eval_num(*children);
        if (c.type == LVAL_ERR) return c;
        r.num *= c.num;
        children++, n--;
    }

    return r;
}

lval eval_div(mpc_ast_t** children, int n) {
    if(n != 2) return lval_err(LERR_BAD_ARITY);

    lval l = eval_num(children[0]);
    if (l.type == LVAL_ERR) return l;
    lval r = eval_num(children[1]);
    if (r.type == LVAL_ERR) return r;

    if (r.num == 0) return lval_err(LERR_DIV_ZERO);

    l.num /= r.num;
    return l;
}

lval eval_mod(mpc_ast_t** children, int n) {
    if(n != 2) return lval_err(LERR_BAD_ARITY);

    lval l = eval_num(children[0]);
    if (l.type == LVAL_ERR) return l;
    lval r = eval_num(children[1]);
    if (r.type == LVAL_ERR) return r;

    if (r.num == 0) return lval_err(LERR_DIV_ZERO);

    l.num %= r.num;
    return l;
}

lval eval_min(mpc_ast_t** children, int n) {
    if(n <= 0) return lval_err(LERR_BAD_ARITY);

    lval c;
    lval r = eval_num(*children);
    if (r.type == LVAL_ERR) return r;
    children++, n--;

    while(n) {
        c = eval_num(*children);
        if (c.type == LVAL_ERR) return c;
        if (c.num < r.num) r = c;
        children++, n--;
    }

    return r;
}

lval eval_max(mpc_ast_t** children, int n) {
    if(n <= 0) return lval_err(LERR_BAD_ARITY);

    lval c;
    lval r = eval_num(*children);
    if (r.type == LVAL_ERR) return r;
    children++, n--;

    while(n) {
        c = eval_num(*children);
        if (c.type == LVAL_ERR) return c;
        if (c.num > r.num) r = c;
        children++, n--;
    }

    return r;
}

lval eval(mpc_ast_t *t) {

    char *op;
    mpc_ast_t** children;
    int n;


    if (strstr(t->tag, "number")) {
        long x = strtol(t->contents, NULL, 10);
        if (errno == ERANGE) return lval_err(LERR_BAD_NUM);
        return lval_num(x);
    }

    op = t->children[1]->contents;
    children = t->children + 2;
    n = t->children_num - 3;

    if (!strcmp(op, "+")) return eval_plus(children, n);
    if (!strcmp(op, "-")) return eval_minus(children, n);
    if (!strcmp(op, "*")) return eval_mul(children, n);
    if (!strcmp(op, "/")) return eval_div(children, n);
    if (!strcmp(op, "\%")) return eval_mod(children, n);
    if (!strcmp(op, "min")) return eval_min(children, n);
    if (!strcmp(op, "max")) return eval_max(children, n);

    assert(0);
}

int main(int argc, char** argv) {
    char* input;
    mpc_result_t mpc_result;
    lval result;

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
            lval_println(result);
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
