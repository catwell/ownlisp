#include "ownlisp.h"

lval * builtin_plus(expr *this) {
    lval *c;
    lval *r = lval_num(0);

    while(this->count) {
        c = expr_pop_num(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        r->num += c->num;
        lval_del(c);
    }

    return r;
}

lval * builtin_minus(expr *this) {
    lval *c;
    lval *r;

    if  (this->count > 1) {
        r = expr_pop_num(this);
    }
    else {
        r = lval_num(0);
    }

    while(this->count) {
        c = expr_pop_num(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        r->num -= c->num;
        lval_del(c);
    }

    return r;
}

lval * builtin_mul(expr *this) {
    lval *c;
    lval *r = lval_num(1);

    while(this->count) {
        c = expr_pop_num(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        r->num *= c->num;
        lval_del(c);
    }

    return r;
}

lval * builtin_div(expr *this) {
    if(this->count != 2) return LERR_BAD_ARITY;

    lval *left = expr_pop_num(this);
    if (left->type == LVAL_ERR) return left;
    lval *right = expr_pop_num(this);
    if (right->type == LVAL_ERR) {
        lval_del(left);
        return right;
    }

    if (right->num == 0) {
        lval_del(left); lval_del(right);
        return LERR_DIV_ZERO;
    }

    left->num /= right->num;
    return left;
}

lval * builtin_mod(expr *this) {
    if(this->count != 2) return LERR_BAD_ARITY;

    lval *left = expr_pop_num(this);
    if (left->type == LVAL_ERR) return left;
    lval *right = expr_pop_num(this);
    if (right->type == LVAL_ERR) {
        lval_del(left);
        return right;
    }

    if (right->num == 0) {
        lval_del(left); lval_del(right);
        return LERR_DIV_ZERO;
    }

    left->num %= right->num;
    return left;
}

lval * builtin_min(expr *this) {
    if(this->count < 1) return LERR_BAD_ARITY;

    lval *c;
    lval *r = expr_pop_num(this);
    if (r->type == LVAL_ERR) return r;

    while(this->count) {
        c = expr_pop_num(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        if(c->num > r->num) {
            lval_del(r);
            r = c;
        }
        else {
            lval_del(c);
        }
    }

    return r;
}

lval * builtin_max(expr *this) {
    if(this->count < 1) return LERR_BAD_ARITY;

    lval *c;
    lval *r = expr_pop_num(this);
    if (r->type == LVAL_ERR) return r;

    while(this->count) {
        c = expr_pop_num(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        if(c->num < r->num) {
            lval_del(r);
            r = c;
        }
        else {
            lval_del(c);
        }
    }

    return r;
}

lval * builtin_head(expr *this) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r = expr_pop_qexpr(this);
    if (r->type == LVAL_ERR) return r;

    if (r->expr->count == 0) {
        lval_del(r);
        return LERR_EMPTY;
    }

    while(r->expr->count > 1) {
        lval_del(expr_pop(r->expr, 1));
    }

    return r;
}

lval * builtin_tail(expr *this) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r = expr_pop_qexpr(this);
    if (r->type == LVAL_ERR) return r;

    if (r->expr->count == 0) {
        lval_del(r);
        return LERR_EMPTY;
    }

    lval_del(expr_pop(r->expr, 0));

    return r;
}

lval * builtin_list(expr *this) {
    lval *r = lval_qexpr();

    while(this->count) {
        lval_append(r, expr_pop(this, 0));
    }

    return r;
}

lval * builtin_eval(expr *this) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r = expr_pop_qexpr(this);
    if (r->type == LVAL_ERR) return r;

    r->type = LVAL_SEXPR;
    return lval_eval(r);
}

lval * builtin_join(expr *this) {
    if(this->count < 1) return LERR_BAD_ARITY;

    lval *c;
    lval *r = expr_pop_qexpr(this);
    if (r->type == LVAL_ERR) return r;

    while(this->count) {
        c = expr_pop_qexpr(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        while(c->expr->count) {
            lval_append(r, expr_pop(c->expr, 0));
        }
        lval_del(c);
    }

    return r;
}

lval * builtin_cons(expr *this) {
    if(this->count != 2) return LERR_BAD_ARITY;

    lval *c = expr_pop(this, 0);
    lval *r = expr_pop_qexpr(this);
    if (c->type == LVAL_ERR) {
        lval_del(c);
        return r;
    }

    lval_prepend(r, c);

    return r;
}

lval * builtin_len(expr *this) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r;
    lval *c = expr_pop_qexpr(this);
    if (c->type == LVAL_ERR) return c;

    r = lval_num(c->expr->count);
    lval_del(c);

    return r;
}

lval * builtin_init(expr *this) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r = expr_pop_qexpr(this);
    if (r->type == LVAL_ERR) return r;

    if (r->expr->count == 0) {
        lval_del(r);
        return LERR_EMPTY;
    }

    lval_del(expr_pop(r->expr, r->expr->count - 1));

    return r;
}

lval * builtin(expr *this) {
    int i;
    lval *head;
    lval *r;

    for(i = 0; i < this->count; ++i) {
        this->cell[i] = lval_eval(this->cell[i]);
        if (this->cell[i]->type == LVAL_ERR) return expr_pop(this, i);
    }

    if (this->count == 0) return lval_sexpr();
    if (this->count == 1) return expr_pop(this, 0);

    head = expr_pop(this, 0);

    if (head->type != LVAL_SYM) r =  LERR_BAD_SEXP;
    else if (!strcmp(head->sym, "+")) r = builtin_plus(this);
    else if (!strcmp(head->sym, "-")) r = builtin_minus(this);
    else if (!strcmp(head->sym, "*")) r = builtin_mul(this);
    else if (!strcmp(head->sym, "/")) r = builtin_div(this);
    else if (!strcmp(head->sym, "\%")) r = builtin_mod(this);
    else if (!strcmp(head->sym, "min")) r = builtin_min(this);
    else if (!strcmp(head->sym, "max")) r = builtin_max(this);
    else if (!strcmp(head->sym, "list")) r = builtin_list(this);
    else if (!strcmp(head->sym, "head")) r = builtin_head(this);
    else if (!strcmp(head->sym, "tail")) r = builtin_tail(this);
    else if (!strcmp(head->sym, "eval")) r = builtin_eval(this);
    else if (!strcmp(head->sym, "join")) r = builtin_join(this);
    else if (!strcmp(head->sym, "cons")) r = builtin_cons(this);
    else if (!strcmp(head->sym, "len")) r = builtin_len(this);
    else if (!strcmp(head->sym, "init")) r = builtin_init(this);
    else r = LERR_BAD_OP;

    lval_del(head);
    return r;
}
