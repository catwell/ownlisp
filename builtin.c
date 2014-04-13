#include "ownlisp.h"


#define BUILTIN_CMP(cmp)                                                       \
    do {                                                                       \
        lval *fst;                                                             \
        lval *cur;                                                             \
                                                                               \
        if(this->count < 2) return LERR_BAD_ARITY;                             \
                                                                               \
        fst = expr_pop(this, 0);                                               \
        if (fst->type == LVAL_ERR) return fst;                                 \
                                                                               \
        while(this->count) {                                                   \
            cur = expr_pop(this, 0);                                           \
            if (cur->type == LVAL_ERR) {                                       \
                lval_del(fst);                                                 \
                return cur;                                                    \
            }                                                                  \
            if (cmp(fst, cur)) {                                               \
                lval_del(fst);                                                 \
                lval_del(cur);                                                 \
                return lval_boolean(0);                                        \
            }                                                                  \
            lval_del(cur);                                                     \
        }                                                                      \
                                                                               \
        lval_del(fst);                                                         \
        return lval_boolean(1);                                                \
    } while(0)

lval * builtin_eq(expr *this, lenv *env) {
    BUILTIN_CMP(!lval_eq);
}

lval * builtin_ne(expr *this, lenv *env) {
    BUILTIN_CMP(lval_eq);
}

#undef BUILTIN_CMP

#define BUILTIN_FOLD(init, op)                                                 \
do {                                                                           \
    lval *c;                                                                   \
    lval *r = lval_num(init);                                                  \
                                                                               \
    while(this->count) {                                                       \
        c = expr_pop_num(this);                                                \
        if (c->type == LVAL_ERR) {                                             \
            lval_del(r);                                                       \
            return c;                                                          \
        }                                                                      \
        r->num op c->num;                                                      \
        lval_del(c);                                                           \
    }                                                                          \
                                                                               \
    return r;                                                                  \
} while(0)

lval * builtin_plus(expr *this, lenv *env) {
    BUILTIN_FOLD(0, +=);
}

lval * builtin_mul(expr *this, lenv *env) {
    BUILTIN_FOLD(1, *=);
}

#undef BUILTIN_FOLD

lval * builtin_minus(expr *this, lenv *env) {
    lval *c;
    lval *r;

    if  (this->count > 1) {
        r = expr_pop_num(this);
        if (r->type == LVAL_ERR) return r;
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

#define BUILTIN_DIV(op)                                                        \
do {                                                                           \
    if(this->count != 2) return LERR_BAD_ARITY;                                \
                                                                               \
    lval *left = expr_pop_num(this);                                           \
    if (left->type == LVAL_ERR) return left;                                   \
    lval *right = expr_pop_num(this);                                          \
    if (right->type == LVAL_ERR) {                                             \
        lval_del(left);                                                        \
        return right;                                                          \
    }                                                                          \
                                                                               \
    if (right->num == 0) {                                                     \
        lval_del(left); lval_del(right);                                       \
        return LERR_DIV_ZERO;                                                  \
    }                                                                          \
                                                                               \
    left->num op right->num;                                                   \
    return left;                                                               \
} while(0)

lval * builtin_div(expr *this, lenv *env) {
    BUILTIN_DIV(/=);
}

lval * builtin_mod(expr *this, lenv *env) {
    BUILTIN_DIV(%=);
}

#undef BUILTIN_DIV

#define BUILTIN_PICK(cmp)                                                      \
do {                                                                           \
    lval *c;                                                                   \
    lval *r;                                                                   \
                                                                               \
    if(this->count < 1) return LERR_BAD_ARITY;                                 \
                                                                               \
    r = expr_pop_num(this);                                                    \
    if (r->type == LVAL_ERR) return r;                                         \
                                                                               \
    while(this->count) {                                                       \
        c = expr_pop_num(this);                                                \
        if (c->type == LVAL_ERR) {                                             \
            lval_del(r);                                                       \
            return c;                                                          \
        }                                                                      \
        if(c->num cmp r->num) {                                                \
            lval_del(r);                                                       \
            r = c;                                                             \
        }                                                                      \
        else {                                                                 \
            lval_del(c);                                                       \
        }                                                                      \
    }                                                                          \
                                                                               \
    return r;                                                                  \
} while(0)


lval * builtin_min(expr *this, lenv *env) {
    BUILTIN_PICK(<);
}

lval * builtin_max(expr *this, lenv *env) {
    BUILTIN_PICK(>);
}

#undef BUILTIN_PICK

#define BUILTIN_ORD(cmp)                                                       \
    do {                                                                       \
        lval *fst;                                                             \
        lval *cur;                                                             \
                                                                               \
        if(this->count < 2) return LERR_BAD_ARITY;                             \
                                                                               \
        fst = expr_pop_num(this);                                              \
        if (fst->type == LVAL_ERR) return fst;                                 \
                                                                               \
        while(this->count) {                                                   \
            cur = expr_pop_num(this);                                          \
            if (cur->type == LVAL_ERR) {                                       \
                lval_del(fst);                                                 \
                return cur;                                                    \
            }                                                                  \
            if (cur->num cmp fst->num) {                                       \
                lval_del(fst);                                                 \
                lval_del(cur);                                                 \
                return lval_boolean(0);                                        \
            }                                                                  \
            lval_del(cur);                                                     \
        }                                                                      \
                                                                               \
        lval_del(fst);                                                         \
        return lval_boolean(1);                                                \
    } while(0)

lval * builtin_lt(expr *this, lenv *env) {
    BUILTIN_ORD(<=);
}

lval * builtin_le(expr *this, lenv *env) {
    BUILTIN_ORD(<);
}

lval * builtin_gt(expr *this, lenv *env) {
    BUILTIN_ORD(>=);
}

lval * builtin_ge(expr *this, lenv *env) {
    BUILTIN_ORD(>);
}

#undef BUILTIN_ORD

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

#define BUILTIN_DEF(setter)                                                    \
do {                                                                           \
    lval *sym;                                                                 \
    lval *v;                                                                   \
                                                                               \
    if(this->count < 1) return LERR_BAD_ARITY;                                 \
                                                                               \
    lval *syms = expr_pop_qexpr(this);                                         \
    if (syms->type == LVAL_ERR) return syms;                                   \
                                                                               \
    if(this->count != syms->expr->count) {                                     \
        lval_del(syms);                                                        \
        return LERR_BAD_ARITY;                                                 \
    }                                                                          \
                                                                               \
    while(this->count) {                                                       \
        sym = expr_pop_sym(syms->expr);                                        \
        if (sym->type == LVAL_ERR) {                                           \
            lval_del(syms);                                                    \
            return sym;                                                        \
        }                                                                      \
        v = expr_pop(this, 0);                                                 \
        setter(env, sym->sym, v);                                              \
    }                                                                          \
                                                                               \
    lval_del(syms);                                                            \
    return lval_sexpr();                                                       \
} while(0)

lval * builtin_def(expr *this, lenv *env) {
    BUILTIN_DEF(lenv_set_global);
}

lval * builtin_deflocal(expr *this, lenv *env) {
    BUILTIN_DEF(lenv_set);
}

#undef BUILTIN_DEF

lval * builtin_lambda(expr *this, lenv *env) {
    int i;
    lambda *r;

    if(this->count != 2) return LERR_BAD_ARITY;

    lval *args = expr_pop_qexpr(this);
    if (args->type == LVAL_ERR) return args;

    lval *body = expr_pop_qexpr(this);
    if (body->type == LVAL_ERR) {
        lval_del(args);
        return body;
    }

    for(i = 0; i < args->expr->count; ++i) {
        if (args->expr->cell[i]->type != LVAL_SYM) {
            lval_del(args);
            lval_del(body);
            return LERR_BAD_TYPE;
        }
    }

    r = lambda_new();
    r->env = lenv_new();
    r->args = args->expr;
    r->body = body->expr;

    args->expr = NULL; body->expr = NULL;
    lval_del(args); lval_del(body);

    return lval_lambda(r);
}

lval * builtin_if(expr *this, lenv *env) {
    lval *b;
    lval *t;
    lval *f;
    lval *r;

    if(this->count != 3) return LERR_BAD_ARITY;
    b = expr_pop_boolean(this);
    if (b->type == LVAL_ERR) return b;
    t = expr_pop_qexpr(this);
    if (t->type == LVAL_ERR) {
        lval_del(b);
        return t;
    }
    f = expr_pop_qexpr(this);
    if (f->type == LVAL_ERR) {
        lval_del(b);
        lval_del(t);
        return f;
    }

    r = expr_eval(b->boolean ? t->expr : f->expr, env);

    lval_del(b);
    lval_del(t);
    lval_del(f);

    return r;
}

lval * builtin_not(expr *this, lenv *env) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r = expr_pop_boolean(this);
    if (r->type == LVAL_ERR) return r;

    r->boolean = 1 - r->boolean;

    return r;
}

#define BUILTIN_FOLD_BOOL(fnd, nfnd)                                           \
do {                                                                           \
    lval *c;                                                                   \
                                                                               \
    while(this->count) {                                                       \
        c = expr_pop_boolean(this);                                            \
        if (c->type == LVAL_ERR) return c;                                     \
        if (c->boolean == fnd) return c;                                       \
        lval_del(c);                                                           \
    }                                                                          \
                                                                               \
    return lval_boolean(nfnd);                                                 \
} while(0)

lval * builtin_and(expr *this, lenv *env) {
    BUILTIN_FOLD_BOOL(0, 1);
}

lval * builtin_or(expr *this, lenv *env) {
    BUILTIN_FOLD_BOOL(1, 0);
}

#undef BUILTIN_FOLD_BOOL

lval * builtin_load(expr *this, lenv *env) {
    lval *fn;
    lval *ast;
    lval *result;
    lval *r;
    mpc_result_t parsed;

    if(this->count != 1) return LERR_BAD_ARITY;

    fn = expr_pop_str(this);
    if (fn->type == LVAL_ERR) return fn;

    if (mpc_parse_contents(fn->str, Lispy, &parsed)) {
        ast = ast_read(parsed.output);
        mpc_ast_delete(parsed.output);
        while (ast->expr->count) {
            result = lval_eval(expr_pop(ast->expr, 0), env);
            if (result->type == LVAL_ERR) lval_println(result);
            lval_del(result);
        }
        lval_del(ast);
        r = lval_sexpr();
    }
    else {
        char *err = mpc_err_string(parsed.error);
        mpc_err_delete(parsed.error);
        r = lval_err(err);
        free(err);
    }

    lval_del(fn);
    return(r);
}

lval * builtin_print(expr *this, lenv *env) {
    lval *cur;

    while(this->count) {
        cur = expr_pop(this, 0);
        lval_print(cur);
        putchar(' ');
        lval_del(cur);
    }
    putchar('\n');

    return lval_sexpr();
}

lval * builtin_error(expr *this, lenv *env) {
    lval *r;

    if(this->count != 1) return LERR_BAD_ARITY;
    r = expr_pop_str(this);
    r->type = LVAL_ERR;

    return r;
}

void register_builtins(lenv *env) {
    lenv_add_builtin(env, "==",    builtin_eq);
    lenv_add_builtin(env, "!=",    builtin_ne);
    lenv_add_builtin(env, "+",     builtin_plus);
    lenv_add_builtin(env, "-",     builtin_minus);
    lenv_add_builtin(env, "*",     builtin_mul);
    lenv_add_builtin(env, "/",     builtin_div);
    lenv_add_builtin(env, "\%",    builtin_mod);
    lenv_add_builtin(env, "min",   builtin_min);
    lenv_add_builtin(env, "max",   builtin_max);
    lenv_add_builtin(env, "<",     builtin_lt);
    lenv_add_builtin(env, "<=",    builtin_le);
    lenv_add_builtin(env, ">",     builtin_gt);
    lenv_add_builtin(env, ">=",    builtin_ge);
    lenv_add_builtin(env, "list",  builtin_list);
    lenv_add_builtin(env, "head",  builtin_head);
    lenv_add_builtin(env, "tail",  builtin_tail);
    lenv_add_builtin(env, "eval",  builtin_eval);
    lenv_add_builtin(env, "join",  builtin_join);
    lenv_add_builtin(env, "cons",  builtin_cons);
    lenv_add_builtin(env, "len",   builtin_len);
    lenv_add_builtin(env, "init",  builtin_init);
    lenv_add_builtin(env, "def",   builtin_def);
    lenv_add_builtin(env, "=",     builtin_deflocal);
    lenv_add_builtin(env, "\\",    builtin_lambda);
    lenv_add_builtin(env, "if",    builtin_if);
    lenv_add_builtin(env, "!",     builtin_not);
    lenv_add_builtin(env, "&&",    builtin_and);
    lenv_add_builtin(env, "||",    builtin_or);
    lenv_add_builtin(env, "load",  builtin_load);
    lenv_add_builtin(env, "print", builtin_print);
    lenv_add_builtin(env, "error", builtin_error);
}
