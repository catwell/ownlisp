#include "ownlisp.h"
mpc_parser_t *Lispy;

lval * ast_read_num(mpc_ast_t *t) {
    assert(strstr(t->tag, "number"));
    long x = strtol(t->contents, NULL, 10);
    if (errno == ERANGE) return LERR_BAD_NUM;
    return lval_num(x);
}

lval * ast_read(mpc_ast_t *t) {
    int i;
    lval *x;

    if (strstr(t->tag, "number")) return ast_read_num(t);

    if (strstr(t->tag, "symbol")) {
        if (!strcmp(t->contents, "true")) {
            return lval_boolean(1);
        }
        else if (!strcmp(t->contents, "false")) {
            return lval_boolean(0);
        }
        else {
            return lval_sym(t->contents);
        }
    }

    if (strstr(t->tag, "string")) {
        ssize_t sz = strlen(t->contents) + 1 - 2 /* quotes */;
        char *unescaped = malloc(sz);
        memcpy(unescaped, t->contents+1, sz);
        unescaped[sz-1] = '\0';
        unescaped = mpcf_unescape(unescaped);
        x = lval_str(unescaped);
        free(unescaped);
        return x;
    }

    if (strstr(t->tag, "comment")) return NULL;

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
        lval *v;
        if (
            (strlen(t->children[i]->contents) == 1) &&
            strstr("(){}", t->children[i]->contents)
        ) continue;
        if (!strcmp(t->children[i]->tag, "regex")) continue;
        v = ast_read(t->children[i]);
        if(v) lval_append(x, v);
    }

    return x;
}

lval * ast_load_eval(char* fn, lenv *env) {
    lval *r;
    mpc_result_t parsed;
    lval *ast;
    lval *result;

    if (mpc_parse_contents(fn, Lispy, &parsed)) {
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

    return r;
}
