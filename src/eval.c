/*
 * Evaluate token lists
 *
 */
/* TODO: tail-recursion (pg 11) */

#include "eval.h"
#include "data.h"
#include "global-state.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Var *call (_Function fn, List args, Environment *hostenv);



/* eval: evaluates a Symbol/List.
 *       for example, given "12.3", returns the Number `12.3',
 *       given the List ('+ '1 '2), returns the List (+ 1 2) */
Var *eval (Var *dat, Environment *env)
{
    /* A list means a function application,
     * so we pass it to call */
    if (dat->type == VAR_LIST)
    {   debug ("list '%v'\ngetting func...", dat);
        Var *func = eval (dat->list.data[0], env);

        if (func->type == VAR_FUNCTION)
        {   debug ("applying '%v'...", func);

            List args;
            args.len =dat->list.len -1;
            args.data=dat->list.data+1;

            return call (func->fn, args, env);
        }
        else
        {   return mkerr_var (EC_BAD_SYNTAX,
                              "Cannot apply '%v'",
                              func);
        }
    }
    /* Identifier */
    else if (dat->type == VAR_IDENTIFIER)
    {
        return id_lookup (env, dat->id);
    }
    return dat;
}

/* call: evaluate a function */
Var *call (_Function fn,
          List args,
          Environment *hostenv)
{
    /* When a LISPFunction (created with `lambda') is called, we
     * must pass the arguments into the function as variables in
     * its environment. To do this, we step through the argument
     * list, and assign the values of each argument to each
     * function local variable. Once this is done, we simply
     * eval() the function body and return the result.
     */

    /* When a BuiltIn is called, the procedure is essentially the
     * same as when a LISPFunction is called, except that BuiltIns
     * can take a potentially infinite number of arguments, or may
     * need to treat the arguments as Symbols, etc. The way this is
     * handled is by passing the arguments as a List, and allowing
     * the BuiltIn to interpret that as it will.
     */

    /* user-defined functions */
    if (fn.type == FN_LISPFN)
    {
        LISPFunction func = fn.fn;

        if (args.len != func.env->len)
        {   return mkerr_var (EC_INVALID_ARG,
                              "%s arguments. Expected %zi, got %zi",
                              (args.len > func.env->len)? "Too many"
                              : "Not enough",
                              func.env->len,
                              args.len);
        }

        /* add the passed args to the function's Environment */
        for (size_t i = 0; i < args.len; ++i)
        {   change_value (func.env,
                          func.env->names[i],
                          eval (args.data[i], hostenv));
        }
        func.env->parent = hostenv;

        local_env = func.env;

        /* call the function */
        debug ("calling function");

        Var *rval = eval (func.body, func.env);

        debug ("returned '%v'", rval);
        local_env = hostenv;
        return rval;
    }
    /* builtin functions */
    else if (fn.type == FN_BUILTIN)
    {
        debug ("got %zi args", args.len);

        Var *result;
        if (fn.builtin.fn)
        {   result = fn.builtin.fn (args, hostenv);
        }
        else
        {   result = mkerr_var (EC_GENERAL,
                                "Operation Not Implemented");
        }
        return result;
    }
    return mkerr_var (EC_GENERAL, "in `%s' -- ???", __func__);
}

