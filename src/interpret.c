/*
 * Low-level interpreter stuff
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "interpret.h"
#include "data.h"
#include "functable.h"
#include "primitive.h"


Data *append (Data *list, int *index, int *len, Data dat);
char *appens (char *str,  int *index, int *len, char chr);


/* print_data: print value of a data */
void print_data (Data d, char endl) {

    switch (d.type) {
    case T_INTEGER:
        printf ("%i%c", d.value.ival, endl);
        break;
    case T_FLOAT:
        printf ("%f%c", d.value.fval, endl);
        break;
    case T_STRING:
        printf ("\"%s\"%c", d.value.str, endl);
        break;
    case T_FUNC:
        printf ("%s%c", d.value.func.name, endl);
        break;
    case T_VARIABLE:
        printf ("%s%c", VAR_TABLE.names[d.value.vari], endl);
        break;
    case T_ERROR:
        printf ("ERROR%c", endl);
        break;
    default:
        printf ("Unknown datatype (%i)%c", d.type, endl);
        break;
    }
}

/* expressionize: convert a string into an s-expression */
Data expressionize (char *string, int end) {

    int  explen       = 0;
    int  expmax       = 3;
    Data *exp         = calloc (3, sizeof(Data) * expmax);
    int  data_i       = 0;

    int  sstr_i       = 0;
    int  sstr_len     = 10;
    char *substring   = calloc (sstr_len, 1);

    static int offset = 0;

    char c, is_define;
    int i;
    is_define = i = 0;

    if (string[0] == '(')
        i = 1;

    for (; i <= end; ++i, ++offset) {

        if (string[i] == ')')
            end = i;

        c = string[i];

        if (!is_define && data_i != 0 && exp[0].type == T_FUNC
         && !strcmp (exp[0].value.func.base, "P_DEF"))
            is_define = 1;

        // eval subexpression + append
        if (c == '(') {
            /*if (is_define) {
                int  decl  = 5;
                char *decs = malloc (decl);
                int s_i = 0;
                for (; string[i-1] != ')' && i < end; ++i)
                    decs = appens (decs, &s_i, &decl, string[i]);
                exp = append (exp, &data_i, &expmax,
                    (Data){ T_STRING, { .str=strdup (decs) } });
                printf ("add %s\n", decs);
            }
            else {*/
                exp = append (exp, &data_i, &expmax,
                    expressionize (string+i+1, end));
                explen++;
            //}
        }
        /* eval substring function
            + clear substring */
        else if ((isspace (c) || c == ')' || i+1 > end)
          && substring[0]) {
            //printf ("add %s\n", substring);

            // check for def'ing
            if (data_i == 1 && is_define) {
                exp = append (exp, &data_i, &expmax,
                    (Data){ T_STRING, { .str=strdup (substring) } });
                explen++;
            }
            else {
                exp = append (exp, &data_i, &expmax,
                    symbol_lookup (substring));
                explen++;
            }

            memset (substring, 0, sstr_len);
            sstr_i = 0;
        }
        // add to substring
        else
            substring = appens (substring, &sstr_i, &sstr_len, c);
    }
    if (i == end-1)
        offset = 0;

    //printf ("%i args: ", data_i);
    //for (int n = 0; n < data_i; ++n)
    //    print_data (exp[n], ' ');
    putchar ('\n');

    free (substring);

    return eval (exp, explen);
}

/* eval: evaluate an s-expression */
Data eval (Data *exp, int len) {

    Data rval = DAT_ERROR;
    int num_args = 0;
    char do_call = 1;

    if (len == 1) {
        rval = exp[0];
        do_call = 0;
    }
    else if (len <= 0) {
        printf ("invalid arg count %i\n", len);
        do_call = 0;
    }
    // error if first value is not a function
    else if (exp[0].type != T_FUNC) {
        printf ("Error: object ");
        print_data (exp[0], ' ');
        puts ("is not a function!");
        do_call = 0;
    }

    // get arg count
    num_args = exp[0].value.func.argc;

    // check for proper arg count
    if (num_args != 0 && len > 0 && len-1 > num_args) {
        error ("Error: invalid no. of arguments - takes %i,"
               "given %i\n", num_args, len);
    }
    else if (do_call)
        rval = call (exp[0].value.func, len, exp+1);

    free (exp);

    return rval;

}

/* call: calls a function */
Data call (struct func func, unsigned int argc, Data *args) {

    char *base = func.base;

    double (*apply)(double, double) = NULL;

    // primitive functions
         if (!strcmp (base, "P_ADD"))
        apply = p_add;
    else if (!strcmp (base, "P_SUB"))
        apply = p_sub;
    else if (!strcmp (base, "P_MUL"))
        apply = p_mul;
    else if (!strcmp (base, "P_DIV"))
        apply = p_div;
    else if (!strcmp (base, "P_DEF")) {
        if (args[0].type == T_STRING) {
            return (Data){ T_VARIABLE, {
                .vari=add_symbol_v (&VAR_TABLE,
                    args[0].value.str, args[1]) } };
        }
        else {
            printf ("Error: call - arg1 (");
            print_data (args[0], 0);
            puts (") is not a name!");
            return DAT_ERROR;
        }
    }

    // apply primitive function
    if (apply) {
        double rval = args[0].type == T_INTEGER? args[0].value.ival :
                        args[0].type == T_FLOAT? args[0].value.fval :
                        printf ("invalid type: %i\n", args[0].type);
        char is_float = 0;
        for (int i = 1; i < argc-1; ++i)
            switch (args[i].type) {
            case T_FLOAT:
                is_float = 1;
                rval = apply (rval, args[i].value.fval);
                break;
            case T_INTEGER:
                rval = apply (rval, args[i].value.ival);
                break;
            case T_VARIABLE:{
                Data dat = VAR_TABLE.data[args[i].value.vari];
                double val = 0;
                switch (dat.type) {
                case T_INTEGER:
                    val = dat.value.ival;
                    break;
                case T_FLOAT:
                    val = dat.value.fval;
                default:
                    error ("Error: call - arg#%i invalid operand"
                    " for %s : ", i+1, base+2);
                    print_data (dat, '\n');
                    return DAT_ERROR;
                    break;
                }
                rval = apply (rval, val); }
            default:
                error ("Error: arg #%i - invalid operand for %s : ",
                    i+1, base+2);
                print_data (args[i], '\n');
                return DAT_ERROR;
                break;
            }
        //printf ("primitive returning %f\n\n", rval);
        if (is_float)
            return (Data){ T_FLOAT,   { .fval=rval } };
        else
            return (Data){ T_INTEGER, { .ival=rval } };
    }

    // create a string
    puts (base);

    Data rval = expressionize (base, strlen (base));

    return rval;
}

/* append: append data to list */
Data *append (Data *list, int *index, int *len, Data dat) {

    if (!list) {
        error ("append - %p is NULL!\n", list);
        return 0;
    }

    Data *temp = NULL;
    while (*index > *len) {
        (*len) *= 2;
        temp = realloc (list, sizeof(Data) * *len);
        while (!temp)
            fatal ("append - failed realloc!\n");
    }
    if (temp) {
        list = temp;
        free (temp);
    }
    list[(*index)++] = dat;

    return list;
}

/* appens: append char to string */
char *appens (char *str, int *index, int *len, char c) {

    if (!str) {
        error ("appens - %p is NULL!\n", str);
        return 0;
    }

    char *temp = NULL;
    while (*index > *len) {
        (*len) *= 2;
        temp = realloc (str, sizeof(char) * *len);
        while (!temp)
            fatal ("appens - failed realloc!\n");
    }
    if (temp) {
        str = temp;
        free (temp);
    }
    str[(*index)++] = c;

    return str;
}

