/*
 * Variables
 *
 */

#ifndef __VAR_H
#define __VAR_H


#include <stdbool.h>    /* bool */



/* Atom:
 *  Primitive data
 */
typedef struct Atom
{   union
    {   bool       boolean;
        double     num;
        String     str;
        String     sym;
        Identifier id;
        Error      err;
        _Function  fn;
    };
    enum AtomType
    {   ATM_BOOLEAN,
        ATM_NUMBER,
        ATM_STRING,
        ATM_SYMBOL,
        ATM_IDENTIFIER,
        ATM_ERROR,
        ATM_FUNCTION,
    } type;
} Atom;

/* Var:
 *  A LISP variable
 */
typedef struct Var
{   union
    {   Atom a;
        Pair p;
    };
    enum VarType
    {   VAR_ATOM,
        VAR_PAIR,
        VAR_NIL,
        VAR_UNDEFINED,
    } type;
} Var;



Var *new_var (enum VarType t);

Var *var_atom (Atom a);
Var *var_pair (Var *car, Var *cdr);
Var *var_nil(void);
Var *var_undefined(void);

Var *var_true(void);
Var *var_false(void);

Atom atm_bool (bool b);
Atom atm_num (double n);
Atom atm_str (String s);
Atom atm_sym (String s);
Atom atm_id (Identifier i);
Atom atm_err (Error e);
Atom atm_fn (_Function f);


#endif

