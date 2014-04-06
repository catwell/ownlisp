#include "ownlisp.h"

/* constructors */

lval * lval_num(long x) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
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

lval * lval_fun(lbuiltin fun) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
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
    int i;

    switch (this->type) {
        case LVAL_ERR:
            free(this->err);
        break;
        case LVAL_NUM:
        break;
        case LVAL_SYM:
            free(this->sym);
        break;
        case LVAL_FUN:
        break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            for (i = 0; i < this->expr->count; ++i) {
                lval_del(this->expr->cell[i]);
            }
            if (this->expr->cell) free(this->expr->cell);
            free(this->expr);
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
        case LVAL_SYM:
            sz = strlen(this->sym) + 1;
            r->sym = malloc(sz);
            memcpy(r->sym, this->sym, sz);
        break;
        case LVAL_FUN:
            r->fun = this->fun;
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

/* print */

void lval_print(lval *this) {
    switch (this->type) {
        case LVAL_ERR:
            printf("ERROR %s\n", this->err);
        break;
        case LVAL_NUM:
            printf("%ld", this->num);
        break;
        case LVAL_SYM:
            printf("%s", this->sym);
        break;
        case LVAL_FUN:
            printf("<function>\n");
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

/* eval */

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
