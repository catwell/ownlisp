#include "ownlisp.h"

lenv * lenv_new(void) {
    lenv *this = malloc(sizeof(lenv));
    this->count = 0;
    this->syms = NULL;
    this->vals = NULL;
    return this;
}

void lenv_del(lenv *this) {
    int i;
    for(i = 0; i < this->count; ++i) {
        free(this->syms[i]);
        lval_del(this->vals[i]);
    }
    free(this->syms);
    free(this->vals);
    free(this);
}

lval * lenv_get(lenv *this, char *sym) {
    int i;
    for(i = 0; i < this->count; ++i) {
        if(!strcmp(this->syms[i], sym)) return lval_copy(this->vals[i]);
    }
    return LERR_UNBOUND;
}

void lenv_put_nocopy(lenv *this, char *sym, lval *v) {
    int i;
    int sz;

    for(i = 0; i < this->count; ++i) {
        if(!strcmp(this->syms[i], sym)) {
            /* already exists, replace */
            lval_del(this->vals[i]);
            this->vals[i] = v;
            sz = strlen(sym) + 1;
            this->syms[i] = realloc(this->syms[i], sz);
            memcpy(this->syms[i], sym, sz);
            return;
        }
    }

    /* not found, insert */
    this->count++;
    this->vals = realloc(this->vals, sizeof(lval*) * this->count);
    this->syms = realloc(this->syms, sizeof(char*) * this->count);
    this->vals[this->count - 1] = v;
    sz = strlen(sym) + 1;
    this->syms[this->count - 1] = malloc(sz);
    memcpy(this->syms[this->count - 1], sym, sz);
}

void lenv_put(lenv *this, char *sym, lval *v) {
    lenv_put_nocopy(this, sym, lval_copy(v));
}

void lenv_add_builtin(lenv *this, char *name, lbuiltin fun) {
    lval *v = lval_fun(fun);
    lenv_put_nocopy(this, name, v);
}
