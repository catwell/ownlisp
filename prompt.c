#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <editline/readline.h>
#include <histedit.h>

#include "mpc.h"

typedef struct lval lval;

typedef struct {
    int count;
    lval **cell;
} sexpr;

struct lval {
    int type;
    union {
        long num;
        char *err;
        char *sym;
        sexpr *sexpr;
    };
};

/* value types */
enum {
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_SEXPR
};

#define LERR_BAD_OP lval_err("bad operator")
#define LERR_DIV_ZERO lval_err("division by 0")
#define LERR_BAD_NUM lval_err("bad number")
#define LERR_BAD_ARITY lval_err("bad arity")
#define LERR_BAD_SEXP lval_err("bad S-Expression")

void lval_print(lval *this);
void sexpr_print(sexpr *this);
lval *lval_eval(lval *this);

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

lval * lval_sexpr(void) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->sexpr = malloc(sizeof(sexpr));
    v->sexpr->count = 0;
    v->sexpr->cell = NULL;
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
        case LVAL_SEXPR:
            for (i = 0; i < this->sexpr->count; ++i) {
                lval_del(this->sexpr->cell[i]);
            }
            if (this->sexpr->cell) free(this->sexpr->cell);
            free(this->sexpr);
        break;
        default:
            assert(0);
    }

    free(this);
}

sexpr *sexpr_add(sexpr *this, lval *x) {
    this->count++;
    this->cell = realloc(this->cell, sizeof(lval*) * this->count);
    this->cell[this->count - 1] = x;
    return this;
}

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

    assert(
        (strcmp(t->tag, ">") == 0) ||
        strstr(t->tag, "sexpr")
    );

    x = lval_sexpr();

    for (i = 0; i < t->children_num; ++i) {
        if (
            (strlen(t->children[i]->contents) == 1) &&
            strstr("(){}", t->children[i]->contents)
        ) continue;
        if (!strcmp(t->children[i]->tag,  "regex")) continue;
        x->sexpr = sexpr_add(x->sexpr, lval_read(t->children[i]));
    }

    return x;
}

void sexpr_print(sexpr *this) {
    int i;

    putchar('(');
    for (i = 0; i < this->count; ++i)
    {
        lval_print(this->cell[i]);
        if (i != this->count - 1) putchar(' ');
    }
    putchar(')');
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
        case LVAL_SEXPR:
            sexpr_print(this->sexpr);
        break;
        default:
            assert(0);
    }
}

void lval_println(lval *this) {
    lval_print(this);
    putchar('\n');
}

lval *sexpr_pop(sexpr *this, int i) {
    lval *r = this->cell[i];
    this->count--;
    memmove(
        this->cell + i, this->cell + i + 1,
        sizeof(lval*) * (this->count - i)
    );
    this->cell = realloc(this->cell, sizeof(lval*) * this->count);
    return r;
}

lval *sexpr_pop_num(sexpr *this) {
    lval *r = sexpr_pop(this, 0);
    if (r->type == LVAL_ERR) return r;
    if (r->type != LVAL_NUM) {
        lval_del(r);
        return LERR_BAD_NUM;
    }
    return r;
}

lval *sexpr_eval_plus(sexpr *this) {
    lval *c;
    lval *r = lval_num(0);

    while(this->count) {
        c = sexpr_pop_num(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        r->num += c->num;
        lval_del(c);
    }

    return r;
}

lval *sexpr_eval_minus(sexpr *this) {
    lval *c;
    lval *r;

    if  (this->count > 1) {
        r = sexpr_pop_num(this);
    }
    else {
        r = lval_num(0);
    }

    while(this->count) {
        c = sexpr_pop_num(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        r->num -= c->num;
        lval_del(c);
    }

    return r;
}

lval *sexpr_eval_mul(sexpr *this) {
    lval *c;
    lval *r = lval_num(1);

    while(this->count) {
        c = sexpr_pop_num(this);
        if (c->type == LVAL_ERR) {
            lval_del(r);
            return c;
        }
        r->num *= c->num;
        lval_del(c);
    }

    return r;
}

lval *sexpr_eval_div(sexpr *this) {
    if(this->count != 2) return LERR_BAD_ARITY;

    lval *left = sexpr_pop_num(this);
    if (left->type == LVAL_ERR) return left;
    lval *right = sexpr_pop_num(this);
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

lval *sexpr_eval_mod(sexpr *this) {
    if(this->count != 2) return LERR_BAD_ARITY;

    lval *left = sexpr_pop_num(this);
    if (left->type == LVAL_ERR) return left;
    lval *right = sexpr_pop_num(this);
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

lval *sexpr_eval_min(sexpr *this) {
    if(this->count < 1) return LERR_BAD_ARITY;

    lval *c;
    lval *r = sexpr_pop_num(this);
    if (r->type == LVAL_ERR) return r;

    while(this->count) {
        c = sexpr_pop_num(this);
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

lval *sexpr_eval_max(sexpr *this) {
    if(this->count < 1) return LERR_BAD_ARITY;

    lval *c;
    lval *r = sexpr_pop_num(this);
    if (r->type == LVAL_ERR) return r;

    while(this->count) {
        c = sexpr_pop_num(this);
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

lval *sexpr_eval(sexpr *this) {
    int i;
    lval *head;
    lval *r;

    for(i = 0; i < this->count; ++i) {
        this->cell[i] = lval_eval(this->cell[i]);
        if (this->cell[i]->type == LVAL_ERR) return sexpr_pop(this, i);
    }

    if (this->count == 0) return lval_sexpr();
    if (this->count == 1) return sexpr_pop(this, 0);

    head = sexpr_pop(this, 0);

    if (head->type != LVAL_SYM) r =  LERR_BAD_SEXP;
    else if (!strcmp(head->sym, "+")) r = sexpr_eval_plus(this);
    else if (!strcmp(head->sym, "-")) r = sexpr_eval_minus(this);
    else if (!strcmp(head->sym, "*")) r = sexpr_eval_mul(this);
    else if (!strcmp(head->sym, "/")) r = sexpr_eval_div(this);
    else if (!strcmp(head->sym, "\%")) r = sexpr_eval_mod(this);
    else if (!strcmp(head->sym, "min")) r = sexpr_eval_min(this);
    else if (!strcmp(head->sym, "max")) r = sexpr_eval_max(this);
    else r = LERR_BAD_OP;

    lval_del(head);
    return r;
}

lval *lval_eval(lval *this) {
    lval *r = this;
    if (this->type == LVAL_SEXPR) {
        r = sexpr_eval(this->sexpr);
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
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(
        MPC_LANG_DEFAULT,
        "number   :  /-?[0-9]+/ ;"
        "symbol   :  '+' | '-' | '*' | '/' | '\%' | \"min\" | \"max\" ;"
        "sexpr    :  '(' <expr>* ')' ;"
        "expr     :  <number> | <symbol> | <sexpr> ;"
        "lispy    :  /^/ <expr>* /$/ ;",
        Number, Symbol, Sexpr, Expr, Lispy
    );

    for(;;) {

        input = readline("> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, Lispy, &mpc_result)) {
            result = lval_read(mpc_result.output);
            lval_println(result);
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

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

    return 0;
}
