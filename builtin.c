#include "ownlisp.h"

lval * builtin_plus(expr *this, lenv *env) {
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

lval * builtin_minus(expr *this, lenv *env) {
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

lval * builtin_mul(expr *this, lenv *env) {
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

lval * builtin_div(expr *this, lenv *env) {
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

lval * builtin_mod(expr *this, lenv *env) {
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

lval * builtin_min(expr *this, lenv *env) {
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

lval * builtin_max(expr *this, lenv *env) {
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

lval * builtin_head(expr *this, lenv *env) {
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

lval * builtin_tail(expr *this, lenv *env) {
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

lval * builtin_list(expr *this, lenv *env) {
    lval *r = lval_qexpr();

    while(this->count) {
        lval_append(r, expr_pop(this, 0));
    }

    return r;
}

lval * builtin_eval(expr *this, lenv *env) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r = expr_pop_qexpr(this);
    if (r->type == LVAL_ERR) return r;

    r->type = LVAL_SEXPR;
    return lval_eval(r, env);
}

lval * builtin_join(expr *this, lenv *env) {
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

lval * builtin_cons(expr *this, lenv *env) {
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

lval * builtin_len(expr *this, lenv *env) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r;
    lval *c = expr_pop_qexpr(this);
    if (c->type == LVAL_ERR) return c;

    r = lval_num(c->expr->count);
    lval_del(c);

    return r;
}

lval * builtin_init(expr *this, lenv *env) {
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

lval * builtin_def(expr *this, lenv *env) {
    lval *sym;
    lval *v;

    if(this->count < 1) return LERR_BAD_ARITY;

    lval *syms = expr_pop_qexpr(this);
    if (syms->type == LVAL_ERR) return syms;

    if(this->count != syms->expr->count) {
        lval_del(syms);
        return LERR_BAD_ARITY;
    }

    while(this->count) {
        sym = expr_pop_sym(syms->expr);
        if (sym->type == LVAL_ERR) {
            lval_del(syms);
            return sym;
        }
        v = expr_pop(this, 0);
        lenv_put_nocopy(env, sym->sym, v);
    }

    lval_del(syms);
    return lval_sexpr();
}

void register_builtins(lenv *env) {
    lenv_add_builtin(env, "+", builtin_plus);
    lenv_add_builtin(env, "-", builtin_minus);
    lenv_add_builtin(env, "*", builtin_mul);
    lenv_add_builtin(env, "/", builtin_div);
    lenv_add_builtin(env, "\%", builtin_mod);
    lenv_add_builtin(env, "min", builtin_min);
    lenv_add_builtin(env, "max", builtin_max);
    lenv_add_builtin(env, "list", builtin_list);
    lenv_add_builtin(env, "head", builtin_head);
    lenv_add_builtin(env, "tail", builtin_tail);
    lenv_add_builtin(env, "eval", builtin_eval);
    lenv_add_builtin(env, "join", builtin_join);
    lenv_add_builtin(env, "cons", builtin_cons);
    lenv_add_builtin(env, "len", builtin_len);
    lenv_add_builtin(env, "init", builtin_init);
    lenv_add_builtin(env, "def", builtin_def);
}
