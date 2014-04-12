#include "ownlisp.h"

lambda * lambda_new(void) {
    lambda *this = malloc(sizeof(lambda));
    this->env = NULL;
    this->args = NULL;
    this->body = NULL;
    return this;
}

void lambda_del(lambda *this) {
    if (this->env) lenv_del(this->env);
    if (this->args) expr_del(this->args);
    if (this->body) expr_del(this->body);
    free(this);
}

lambda * lambda_copy(lambda *this) {
    lambda *r = malloc(sizeof(lambda));

    r->env = lenv_copy(this->env);
    r->args = expr_copy(this->args);
    r->body = expr_copy(this->body);

    return r;
}

lval * lambda_call(lambda *this, expr *args, lenv *env) {
    while (args->count) { /* bind arguments */
        if (this->args->count == 0) {
            return LERR_BAD_ARITY;
        }
        lval *v;
        lval *sym = expr_pop_sym(this->args);
        assert(sym->type == LVAL_SYM); /* checked earlier */
        if (!strcmp(sym->sym, "&")) { /* variadic */
            lval_del(sym);
            sym = expr_pop_sym(this->args);
            if (sym->type == LVAL_ERR) {
                lval_del(sym);
                return LERR_BAD_FUN;
            }
            v = lval_qexpr();
            free(v->expr);
            v->expr = expr_copy(args);
            lenv_set(this->env, sym->sym, v);
            lval_del(sym);
            break;
        }
        v = expr_pop(args, 0);
        lenv_set(this->env, sym->sym, v);
        lval_del(sym);
    }
    if (
        (this->args->count > 0) &&
        (!strcmp(this->args->cell[0]->sym, "&"))
    ) { /* variadic part empty */
        lval *sym = expr_pop_sym(this->args);
        if (sym->type == LVAL_ERR) {
            lval_del(sym);
            return LERR_BAD_FUN;
        }
        lval_del(sym);
        sym = expr_pop_sym(this->args);
        if (sym->type == LVAL_ERR) {
            lval_del(sym);
            return LERR_BAD_FUN;
        }
        lenv_set(this->env, sym->sym, lval_qexpr());
        lval_del(sym);
    }
    if (this->args->count == 0) { /* evaluate */
        this->env->parent = env;
        lval *f = lval_sexpr();
        free(f->expr);
        f->expr = expr_copy(this->body);
        return lval_eval(f, this->env);
    }
    else { /* return partial */
        return lval_lambda(lambda_copy(this));
    }
}

void lambda_print(lambda *this) {
    /* TODO print value of bound symbols */
    printf("(\\ ");
    expr_print(this->args, '{', '}');
    putchar(' ');
    expr_print(this->body, '{', '}');
    putchar(')');
}
