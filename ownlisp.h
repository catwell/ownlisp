#ifndef OWNLISP_H
#define OWNLISP_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>

#include "mpc.h"

#define DEBUG 0

typedef struct lval lval;
typedef struct  lenv lenv;

typedef lval * (*lbuiltin)(lenv*, lval*);

typedef struct {
    int count;
    lval **cell;
} expr;

struct lval {
    int type;
    union {
        long num;
        char *err;
        char *sym;
        expr *expr;
        lbuiltin fun;
    };
};

struct lenv
{
    int count;
    char **syms;
    lval **vals;
};

/* value types */
enum {
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_FUN,
    LVAL_SEXPR,
    LVAL_QEXPR
};

/* expr */

expr * expr_copy(expr *this);
expr * expr_append(expr *this, lval *x);
expr * expr_prepend(expr *this, lval *x);
void expr_print(expr *this, char open, char close);
lval * expr_pop(expr *this, int i);
lval * expr_pop_typed(expr *this, int type);

#define expr_pop_num(this) expr_pop_typed((this), LVAL_NUM)
#define expr_pop_qexpr(this) expr_pop_typed((this), LVAL_QEXPR)

/* lval */

lval * lval_num(long x);
lval * lval_err(char *x);
lval * lval_sym(char *x);
lval * lval_fun(lbuiltin fun);
lval * lval_sexpr(void);
lval * lval_qexpr(void);

void lval_del(lval *this);
lval * lval_copy(lval *this);
void lval_print(lval *this);
void lval_println(lval *this);
lval * lval_eval(lval *this);

#define lval_append(this, x) (this)->expr = expr_append((this)->expr, (x))
#define lval_prepend(this, x) (this)->expr = expr_prepend((this)->expr, (x))

/* ast */

lval * ast_read_num(mpc_ast_t *t);
lval * ast_read(mpc_ast_t *t);

/* builtin */

lval * builtin_plus(expr *this);
lval * builtin_minus(expr *this);
lval * builtin_mul(expr *this);
lval * builtin_div(expr *this);
lval * builtin_mod(expr *this);
lval * builtin_min(expr *this);
lval * builtin_max(expr *this);
lval * builtin_head(expr *this);
lval * builtin_tail(expr *this);
lval * builtin_list(expr *this);
lval * builtin_eval(expr *this);
lval * builtin_join(expr *this);
lval * builtin_cons(expr *this);
lval * builtin_len(expr *this);
lval * builtin_init(expr *this);

lval * builtin(expr *this);

/* errors */

#define LERR_BAD_OP lval_err("bad operator")
#define LERR_DIV_ZERO lval_err("division by 0")
#define LERR_BAD_NUM lval_err("bad number")
#define LERR_BAD_ARITY lval_err("bad arity")
#define LERR_BAD_SEXP lval_err("bad S-Expression")
#define LERR_BAD_TYPE lval_err("bad type")
#define LERR_EMPTY lval_err("empty")

#endif /* OWNLISP_H */
