#include "ownlisp.h"

lenv * lenv_new(void) {
    lenv *this = malloc(sizeof(lenv));
    this->parent = NULL;
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

lenv * lenv_copy(lenv *this) {
    int i;
    int sz;

    lenv *r = malloc(sizeof(lenv));

    r->count = this->count;
    r->parent = this->parent;
    r->syms = malloc(sizeof(char*) * r->count);
    r->vals = malloc(sizeof(lval*) * r->count);

    for(i = 0; i < r->count; ++i) {
        sz = strlen(this->syms[i]) + 1;
        r->syms[i] = malloc(sz);
        memcpy(r->syms[i], this->syms[i], sz);
        r->vals[i] = lval_copy(this->vals[i]);
    }

    return r;
}

lval * lenv_get(lenv *this, char *sym) {
    int i;
    for(i = 0; i < this->count; ++i) {
        if(!strcmp(this->syms[i], sym)) return lval_copy(this->vals[i]);
    }
    if (this->parent) return lenv_get(this->parent, sym);
    return LERR_UNBOUND;
}

void lenv_set(lenv *this, char *sym, lval *v) {
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

void lenv_set_global(lenv *this, char *sym, lval *v) {
    while (this->parent) this = this->parent;
    lenv_set(this, sym, v);
}

void lenv_add_builtin(lenv *this, char *name, lbuiltin builtin) {
    lval *v = lval_builtin(builtin);
    lenv_set(this, name, v);
}
