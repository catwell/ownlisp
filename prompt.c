#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <editline/readline.h>
#include <histedit.h>

#include "mpc.h"

#define DEBUG 0

typedef struct lval lval;
typedef struct  lenv lenv;

typedef lval * (*lbuiltin)(lenv*, lval*);

typedef struct {
    int count;
    lval **cell;
} expr;

struct lval {
    int type;
    union {
        long num;
        char *err;
        char *sym;
        expr *expr;
        lbuiltin fun;
    };
};

struct lenv
{
    int count;
    char **syms;
    lval **vals;
};

/* value types */
enum {
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_FUN,
    LVAL_SEXPR,
    LVAL_QEXPR
};

#define LERR_BAD_OP lval_err("bad operator")
#define LERR_DIV_ZERO lval_err("division by 0")
#define LERR_BAD_NUM lval_err("bad number")
#define LERR_BAD_ARITY lval_err("bad arity")
#define LERR_BAD_SEXP lval_err("bad S-Expression")
#define LERR_BAD_TYPE lval_err("bad type")
#define LERR_EMPTY lval_err("empty")

void lval_print(lval *this);
lval *lval_eval(lval *this);
lval * lval_copy(lval *this);

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

expr *expr_append(expr *this, lval *x) {
    this->count++;
    this->cell = realloc(this->cell, sizeof(lval*) * this->count);
    this->cell[this->count - 1] = x;
    return this;
}

expr *expr_prepend(expr *this, lval *x) {
    int i;
    this->count++;
    this->cell = realloc(this->cell, sizeof(lval*) * this->count);
    for(i = this->count - 1; i > 0; --i) {
        this->cell[i] = this->cell[i-1];
    }
    this->cell[0] = x;
    return this;
}

#define lval_append(this, x) (this)->expr = expr_append((this)->expr, (x))
#define lval_prepend(this, x) (this)->expr = expr_prepend((this)->expr, (x))

lval *lval_read_num(mpc_ast_t *t) {
    assert(strstr(t->tag, "number"));
    long x = strtol(t->contents, NULL, 10);
    if (errno == ERANGE) return LERR_BAD_NUM;
    return lval_num(x);
}

lval *lval_read(mpc_ast_t *t) {
    int i;
    lval *x;

    if (strstr(t->tag, "number")) return lval_read_num(t);
    if (strstr(t->tag, "symbol")) return lval_sym(t->contents);


    if (strstr(t->tag, "qexpr")) {
        x = lval_qexpr();
    }
    else {
        assert(
            (strcmp(t->tag, ">") == 0) ||
            strstr(t->tag, "sexpr")
        );
        x = lval_sexpr();
    }

    for (i = 0; i < t->children_num; ++i) {
        if (
            (strlen(t->children[i]->contents) == 1) &&
            strstr("(){}", t->children[i]->contents)
        ) continue;
        if (!strcmp(t->children[i]->tag,  "regex")) continue;
        lval_append(x, lval_read(t->children[i]));
    }

    return x;
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

lval *expr_pop(expr *this, int i) {
    lval *r = this->cell[i];
    this->count--;
    memmove(
        this->cell + i, this->cell + i + 1,
        sizeof(lval*) * (this->count - i)
    );
    this->cell = realloc(this->cell, sizeof(lval*) * this->count);
    return r;
}

lval *expr_pop_typed(expr *this, int type) {
    lval *r = expr_pop(this, 0);
    if (r->type == LVAL_ERR) return r;
    if (r->type != type) {
        lval_del(r);
        return LERR_BAD_TYPE;
    }
    return r;
}

#define expr_pop_num(this) expr_pop_typed((this), LVAL_NUM)
#define expr_pop_qexpr(this) expr_pop_typed((this), LVAL_QEXPR)

lval *sexpr_eval_plus(expr *this) {
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

lval *sexpr_eval_minus(expr *this) {
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

lval *sexpr_eval_mul(expr *this) {
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

lval *sexpr_eval_div(expr *this) {
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

lval *sexpr_eval_mod(expr *this) {
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

lval *sexpr_eval_min(expr *this) {
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

lval *sexpr_eval_max(expr *this) {
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

lval *sexpr_eval_head(expr *this) {
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

lval *sexpr_eval_tail(expr *this) {
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

lval *sexpr_eval_list(expr *this) {
    lval *r = lval_qexpr();

    while(this->count) {
        lval_append(r, expr_pop(this, 0));
    }

    return r;
}

lval *sexpr_eval_eval(expr *this) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r = expr_pop_qexpr(this);
    if (r->type == LVAL_ERR) return r;

    r->type = LVAL_SEXPR;
    return lval_eval(r);
}

lval *sexpr_eval_join(expr *this) {
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

lval *sexpr_eval_cons(expr *this) {
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

lval *sexpr_eval_len(expr *this) {
    if(this->count != 1) return LERR_BAD_ARITY;

    lval *r;
    lval *c = expr_pop_qexpr(this);
    if (c->type == LVAL_ERR) return c;

    r = lval_num(c->expr->count);
    lval_del(c);

    return r;
}

lval *sexpr_eval_init(expr *this) {
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

lval *sexpr_eval(expr *this) {
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
    else if (!strcmp(head->sym, "+")) r = sexpr_eval_plus(this);
    else if (!strcmp(head->sym, "-")) r = sexpr_eval_minus(this);
    else if (!strcmp(head->sym, "*")) r = sexpr_eval_mul(this);
    else if (!strcmp(head->sym, "/")) r = sexpr_eval_div(this);
    else if (!strcmp(head->sym, "\%")) r = sexpr_eval_mod(this);
    else if (!strcmp(head->sym, "min")) r = sexpr_eval_min(this);
    else if (!strcmp(head->sym, "max")) r = sexpr_eval_max(this);
    else if (!strcmp(head->sym, "list")) r = sexpr_eval_list(this);
    else if (!strcmp(head->sym, "head")) r = sexpr_eval_head(this);
    else if (!strcmp(head->sym, "tail")) r = sexpr_eval_tail(this);
    else if (!strcmp(head->sym, "eval")) r = sexpr_eval_eval(this);
    else if (!strcmp(head->sym, "join")) r = sexpr_eval_join(this);
    else if (!strcmp(head->sym, "cons")) r = sexpr_eval_cons(this);
    else if (!strcmp(head->sym, "len")) r = sexpr_eval_len(this);
    else if (!strcmp(head->sym, "init")) r = sexpr_eval_init(this);
    else r = LERR_BAD_OP;

    lval_del(head);
    return r;
}

lval *lval_eval(lval *this) {
    lval *r = this;
    if (this->type == LVAL_SEXPR) {
        r = sexpr_eval(this->expr);
        lval_del(this);
    }
    return r;
}

int main(int argc, char** argv) {
    char* input;
    mpc_result_t mpc_result;
    lval *result;

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Qexpr = mpc_new("qexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(
        MPC_LANG_DEFAULT,
        "number   :  /-?[0-9]+/ ;"
        "symbol   :  /[a-zA-Z0-9_+\\-*\\/\%\\\\=<>!&]+/ ;"
        "sexpr    :  '(' <expr>* ')' ;"
        "qexpr    :  '{' <expr>* '}' ;"
        "expr     :  <number> | <symbol> | <sexpr> | <qexpr> ;"
        "lispy    :  /^/ <expr>* /$/ ;",
        Number, Symbol, Sexpr, Qexpr, Expr, Lispy
    );

    for(;;) {

        input = readline("> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, Lispy, &mpc_result)) {
            result = lval_read(mpc_result.output);
            if(DEBUG) lval_println(result);
            result = lval_eval(result);
            lval_println(result);
            lval_del(result);
            mpc_ast_delete(mpc_result.output);
        }
        else {
            mpc_err_print(mpc_result.error);
            mpc_err_delete(mpc_result.error);
        }

        free(input);
    }

    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}
