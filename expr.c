#include "ownlisp.h"

void expr_del(expr *this) {
    int i;
    for (i = 0; i < this->count; ++i) {
        lval_del(this->cell[i]);
    }
    if (this->cell) free(this->cell);
    free(this);
}

expr * expr_copy(expr *this) {
    int i;
    expr *r = malloc(sizeof(expr));
    r->count = this->count;
    r->cell = malloc(sizeof(lval*) * r->count);
    for(i = 0; i < r->count; ++i) {
        r->cell[i]  = lval_copy(this->cell[i]);
    }
    return r;
}

expr * expr_append(expr *this, lval *x) {
    this->count++;
    this->cell = realloc(this->cell, sizeof(lval*) * this->count);
    this->cell[this->count - 1] = x;
    return this;
}

expr * expr_prepend(expr *this, lval *x) {
    int i;
    this->count++;
    this->cell = realloc(this->cell, sizeof(lval*) * this->count);
    for(i = this->count - 1; i > 0; --i) {
        this->cell[i] = this->cell[i-1];
    }
    this->cell[0] = x;
    return this;
}

void expr_print(expr *this, char open, char close) {
    int i;

    putchar(open);
    for (i = 0; i < this->count; ++i)
    {
        lval_print(this->cell[i]);
        if (i != this->count - 1) putchar(' ');
    }
    putchar(close);
}

lval * expr_pop(expr *this, int i) {
    lval *r = this->cell[i];
    this->count--;
    memmove(
        this->cell + i, this->cell + i + 1,
        sizeof(lval*) * (this->count - i)
    );
    this->cell = realloc(this->cell, sizeof(lval*) * this->count);
    return r;
}

lval * expr_pop_typed(expr *this, int type) {
    lval *r = expr_pop(this, 0);
    if (r->type == LVAL_ERR) return r;
    if (r->type != type) {
        lval_del(r);
        return LERR_BAD_TYPE;
    }
    return r;
}

lval * expr_eval(expr *this, lenv *env) {
    int i;
    lval *head;

    for(i = 0; i < this->count; ++i) {
        this->cell[i] = lval_eval(this->cell[i], env);
        if (this->cell[i]->type == LVAL_ERR) return expr_pop(this, i);
    }

    if (this->count == 0) return lval_sexpr();
    if (this->count == 1) return expr_pop(this, 0);

    head = expr_pop(this, 0);

    return lval_call(head, this, env);
}
