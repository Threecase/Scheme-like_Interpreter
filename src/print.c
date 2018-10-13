/*
 * Printf extension for Vars, Tokens, etc.
 *
 */

#include "print.h"

#include <stdio.h>
#include <printf.h>



/* print_var: printf functionality for LISP Vars */
/* TODO: support formatting stuff like `%6.2v' */
int print_var (FILE *stream,
               const struct printf_info *info,
               const void *const *args)
{
    /* we get passed a Var pointer by printf */
    Var *_v = (*(Var **)(args[0]));
    //printf ("print_var got %p\n", _v);
    Var v = *_v;
    switch (v.type)
    {
    case VAR_NUMBER:
        return fprintf (stream, "%g", v.number);
        break;

    case VAR_STRING:
        return fprintf (stream, "\"%s\"", v.str.chars);
        break;

    case VAR_FUNCTION:
        if (v.fn.type == FN_LISPFN)
        {
            return fprintf (stream, "%p", v.fn.fn.body);
        }
        else if (v.fn.type == FN_BUILTIN)
        {
            return fprintf (stream, "%p", v.fn.builtin.fn);
        }
        break;

    case VAR_LIST:
     {  int len = 2;
        fputc ('(', stream);
        for (size_t i = 0; i < v.list.len-1; ++i)
        {
            len += fprintf (stream, "%v", &v.list.data[i]) + 1;
            fputc (' ', stream);
        }
        if (v.list.len > 0)
        {   len += fprintf (stream, "%v",
                            &v.list.data[v.list.len-1]);
        }
        fputc (')', stream);
        return len;
     }  break;

    case VAR_ERROR:
        return fprintf (stream, "[%s - %s]",
                        err_msg (v.err),
                        v.err.flavour);
        break;

    case VAR_SYMBOL:
        return fprintf (stream, "%s",
                        v.sym.chars);
        break;
    }
    error ("Unknown Var type '%i'", v.type);
    return -1;
}

/* print_token: printf functionality for Tokens */
int print_token (FILE *stream,
                 const struct printf_info *info,
                 const void *const *args)
{
    /* we get passed a Token pointer by printf */
    Token *_t = (*(Token **)(args[0]));
    //printf ("print_token got %p\n", _t);
    Token t = *_t;
    switch (t.type)
    {
    case TOK_NUMBER:
        return fprintf (stream, "%g", t.number);
        break;

    case TOK_STRING:
        return fprintf (stream, "%s", t.str.chars);
        break;

    case TOK_IDENTIFIER:
        return fprintf (stream, "%s", t.id.chars);
        break;

    case TOK_EXPR:
     {  int len = 2;
        fputc ('(', stream);
        size_t i = 0;
        for (; t.subexpr[i+1].type != TOK_ENDEXPR; ++i)
        {
            len += fprintf (stream, "%T", &t.subexpr[i]);
            fputc (' ', stream);
        }
        len += fprintf (stream, "%T", &t.subexpr[i]);
        fputc (')', stream);
        return len;
     }  break;

    case TOK_ENDEXPR:
        return fprintf (stream, "ENDEXPR");
        break;
    }
    error ("Unknown Token type '%i'", t.type);
    return -1;
}

/* print_var_arginfo: used by printf to pass us
 *                    input in the correct form;
 *                    in this case, a pointer */
int print_var_arginfo (const struct printf_info *info,
                       size_t n,
                       int *argtypes)
{   if (n > 0)
    {   argtypes[0] = PA_POINTER;
    }
    return 1;
}

/* print_token_arginfo: used by printf to pass us
 *                      input in the correct form;
 *                      in this case, a pointer */
int print_token_arginfo (const struct printf_info *info,
                         size_t n,
                         int *argtypes)
{   if (n > 0)
    {   argtypes[0] = PA_POINTER;
    }
    return 1;
}


/* init_print: setup custom printf handlers */
void init_print(void)
{   register_printf_function ('v', print_var, print_var_arginfo);
    register_printf_function ('T', print_token, print_token_arginfo);
}

