#include "ownlisp.h"

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
        lval_append(x, ast_read(t->children[i]));
    }

    return x;
}
