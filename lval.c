#include "ownlisp.h"

/* constructors */

lval * lval_num(long x) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval * lval_boolean(int x) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_BOOLEAN;
    v->boolean = x ? 1 : 0;
    return v;
}

lval * lval_err(char *x) {
    ssize_t sz = strlen(x) + 1;
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(sz);
    memcpy(v->err, x, sz);
    return v;
}

lval * lval_sym(char *x) {
    ssize_t sz = strlen(x) + 1;
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(sz);
    memcpy(v->sym, x, sz);
    return v;
}

lval * lval_str(char *x) {
    ssize_t sz = strlen(x) + 1;
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->str = malloc(sz);
    memcpy(v->sym, x, sz);
    return v;
}

lval * lval_builtin(lbuiltin builtin) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_BUILTIN;
    v->builtin = builtin;
    return v;
}

lval * lval_lambda(lambda *fun) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_LAMBDA;
    v->fun = fun;
    return v;
}

lval * lval_sexpr(void) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->expr = malloc(sizeof(expr));
    v->expr->count = 0;
    v->expr->cell = NULL;
    return v;
}

lval * lval_qexpr(void) {
    lval *v = lval_sexpr();
    v->type = LVAL_QEXPR;
    return v;
}

/* destructor, copy */

void lval_del(lval *this) {
    switch (this->type) {
        case LVAL_ERR:
            if(this->err) free(this->err);
        break;
        case LVAL_NUM:
        case LVAL_BOOLEAN:
        break;
        case LVAL_SYM:
            if(this->sym) free(this->sym);
        break;
        case LVAL_STR:
            if(this->str) free(this->str);
        break;
        case LVAL_BUILTIN:
        break;
        case LVAL_LAMBDA:
            if(this->fun) lambda_del(this->fun);
        break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            if(this->expr) expr_del(this->expr);
        break;
        default:
            assert(0);
    }
    free(this);
}

lval * lval_copy(lval *this) {
    ssize_t sz;

    lval *r = malloc(sizeof(lval));
    r->type = this->type;

    switch(this->type) {
        case LVAL_ERR:
            sz = strlen(this->err) + 1;
            r->err = malloc(sz);
            memcpy(r->err, this->err, sz);
        break;
        case LVAL_NUM:
            r->num = this->num;
        break;
        case LVAL_BOOLEAN:
            r->boolean = this->boolean;
        break;
        case LVAL_SYM:
            sz = strlen(this->sym) + 1;
            r->sym = malloc(sz);
            memcpy(r->sym, this->sym, sz);
        break;
        case LVAL_STR:
            sz = strlen(this->str) + 1;
            r->str = malloc(sz);
            memcpy(r->str, this->str, sz);
        break;
        case LVAL_BUILTIN:
            r->builtin = this->builtin;
        break;
        case LVAL_LAMBDA:
            r->fun = lambda_copy(this->fun);
        break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            r->expr = expr_copy(this->expr);
        break;
        default:
            assert(0);
    }

    return r;
}

/* print, eq */

void lval_print(lval *this) {
    ssize_t sz;
    char *escaped;

    switch (this->type) {
        case LVAL_ERR:
            printf("ERROR %s\n", this->err);
        break;
        case LVAL_NUM:
            printf("%ld", this->num);
        break;
        case LVAL_BOOLEAN:
            printf(this->boolean ? "true" : "false");
        break;
        case LVAL_SYM:
            printf("%s", this->sym);
        break;
        case LVAL_STR:
            sz = strlen(this->str) + 1;
            escaped = malloc(sz);
            memcpy(escaped, this->str, sz);
            escaped = mpcf_escape(escaped);
            printf("\"%s\"", escaped);
            free(escaped);
        break;
        case LVAL_BUILTIN:
            printf("<builtin>");
        break;
        case LVAL_LAMBDA:
            lambda_print(this->fun);
        break;
        case LVAL_SEXPR:
            expr_print(this->expr, '(', ')');
        break;
        case LVAL_QEXPR:
            expr_print(this->expr, '{', '}');
        break;
        default:
            assert(0);
    }
}

void lval_println(lval *this) {
    lval_print(this);
    putchar('\n');
}

int lval_eq(lval *x, lval* y) {
    if(x->type != y->type) return 0;

    switch (x->type) {
        case LVAL_ERR:
            return (!strcmp(x->err, y->err));
        case LVAL_NUM:
            return (x->num == y->num);
        case LVAL_BOOLEAN:
            return (x->boolean == y->boolean);
        case LVAL_SYM:
            return (!strcmp(x->sym, y->sym));
        case LVAL_STR:
            return (!strcmp(x->str, y->str));
        case LVAL_BUILTIN:
            return (x->builtin == y->builtin);
        case LVAL_LAMBDA:
            return lambda_eq(x->fun, y->fun);
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            return expr_eq(x->expr, y->expr);
        break;
        default:
            assert(0);
    }
}

char * lval_type(lval *this) {
    switch (this->type) {
        case LVAL_ERR:
            return "error";
        case LVAL_NUM:
            return "number";
        case LVAL_BOOLEAN:
            return "boolean";
        case LVAL_SYM:
            return "symbol";
        case LVAL_STR:
            return "string";
        case LVAL_BUILTIN:
            return "builtin";
        case LVAL_LAMBDA:
            return "lambda";
        case LVAL_SEXPR:
            return "sexpr";
        case LVAL_QEXPR:
            return "qexpr";
        default:
            assert(0);
    }
}

/* call, eval */

lval * lval_call(lval *this, expr *args, lenv *env) {
    lval *r;

    switch (this->type) {
        case LVAL_BUILTIN:
            r = this->builtin(args, env);
        break;
        case LVAL_LAMBDA:
            r = lambda_call(this->fun, args, env);
        break;
        case LVAL_SYM:
            r = LERR_BAD_OP;
        break;
        default:
            r = LERR_BAD_SEXP;
    }

    lval_del(this);
    return r;
}

lval * lval_eval(lval *this, lenv *env) {
    lval *r = this;
    if (this->type == LVAL_SEXPR) {
        r = expr_eval(this->expr, env);
        lval_del(this);
    }
    else if (this->type == LVAL_SYM) {
        r = lenv_get(env, this->sym);
        lval_del(this);
    }
    return r;
}
