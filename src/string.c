/*
 * String functions
 *
 */

#include "data.h"

#include <stdlib.h>
#include <string.h>



/* mkstring: create a string from a char list */
String mkstring (char const *const str)
{
    String r;

    r.chars = strdup (str);
    r.len   = strlen (str);
    r.size  = r.len + 1;

    return r;
}

/* stringdup: duplicate a string */
String stringdup (String s)
{   return mkstring (s.chars);
}

/* free_string:  */
void free_string (String s)
{   free (s.chars);
}

/* stringcmp: compare two strings */
int stringcmp (String a, String b)
{
    if (a.chars != NULL && b.chars != NULL)
    {   /* use the length of the shortest string */
        size_t n = (a.len > b.len)? a.len : b.len;
        return strncmp (a.chars, b.chars, n);
    }
    else
    {   error ("%s failed -- string %s is NULL!",
               __func__,
               (a.chars == NULL)? "a" : "b");
        return -1;
    }
}
