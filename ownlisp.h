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
typedef struct expr expr;
typedef struct lambda lambda;

typedef lval * (*lbuiltin)(expr *this, lenv *env);

struct expr {
    int count;
    lval **cell;
};

struct lambda {
    lenv *env;
    expr *args;
    expr *body;
};

struct lval {
    int type;
    union {
        long num;
        char boolean;
        char *err;
        char *sym;
        expr *expr;
        lbuiltin builtin;
        lambda *fun;
    };
};

struct lenv
{
    lenv *parent;
    int count;
    char **syms;
    lval **vals;
};

/* value types */
enum {
    LVAL_ERR,
    LVAL_NUM,
    LVAL_BOOLEAN,
    LVAL_SYM,
    LVAL_BUILTIN,
    LVAL_LAMBDA,
    LVAL_SEXPR,
    LVAL_QEXPR
};

/* expr */

void expr_del(expr *this);
expr * expr_copy(expr *this);
expr * expr_append(expr *this, lval *x);
expr * expr_prepend(expr *this, lval *x);
void expr_print(expr *this, char open, char close);
lval * expr_pop(expr *this, int i);
lval * expr_pop_typed(expr *this, int type);
lval * expr_eval(expr *this, lenv *env);
int expr_eq(expr *x, expr *y);

#define expr_pop_num(this) expr_pop_typed((this), LVAL_NUM)
#define expr_pop_boolean(this) expr_pop_typed((this), LVAL_BOOLEAN)
#define expr_pop_sym(this) expr_pop_typed((this), LVAL_SYM)
#define expr_pop_qexpr(this) expr_pop_typed((this), LVAL_QEXPR)

/* lval */

lval * lval_num(long x);
lval * lval_boolean(int x);
lval * lval_err(char *x);
lval * lval_sym(char *x);
lval * lval_builtin(lbuiltin builtin);
lval * lval_lambda(lambda *fun);
lval * lval_sexpr(void);
lval * lval_qexpr(void);

void lval_del(lval *this);
lval * lval_copy(lval *this);
void lval_print(lval *this);
void lval_println(lval *this);
int lval_eq(lval *x, lval* y);
lval * lval_call(lval *this, expr *args, lenv *env);
lval * lval_eval(lval *this, lenv *env);

#define lval_append(this, x) (this)->expr = expr_append((this)->expr, (x))
#define lval_prepend(this, x) (this)->expr = expr_prepend((this)->expr, (x))

/* lenv */

lenv * lenv_new(void);
void lenv_del(lenv *this);
lenv * lenv_copy(lenv *this);
lval * lenv_get(lenv *this, char *sym);
void lenv_set(lenv *this, char *sym, lval *v);
void lenv_set_global(lenv *this, char *sym, lval *v);
void lenv_add_builtin(lenv *this, char *name, lbuiltin builtin);

/* lambda */

lambda * lambda_new(void);
void lambda_del(lambda *this);
lambda * lambda_copy(lambda *this);
lval * lambda_call(lambda *this, expr *args, lenv *env);
void lambda_print(lambda *this);
int lambda_eq(lambda *x, lambda *y);

/* ast */

lval * ast_read_num(mpc_ast_t *t);
lval * ast_read(mpc_ast_t *t);

/* builtin */

void register_builtins(lenv *env);

/* errors */

#define LERR_BAD_OP lval_err("bad operator")
#define LERR_DIV_ZERO lval_err("division by 0")
#define LERR_BAD_NUM lval_err("bad number")
#define LERR_BAD_BOOLEAN lval_err("bad boolean")
#define LERR_BAD_ARITY lval_err("bad arity")
#define LERR_BAD_FUN lval_err("bad function definition")
#define LERR_BAD_SEXP lval_err("bad S-Expression")
#define LERR_BAD_TYPE lval_err("bad type")
#define LERR_EMPTY lval_err("empty")
#define LERR_UNBOUND lval_err("unbound symbol")
#define LERR_OVERFLOW lval_err("overflow")

#endif /* OWNLISP_H */
