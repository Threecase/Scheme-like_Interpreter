/*
 * Strings
 *
 */

#ifndef __STRING_H
#define __STRING_H

#include <stddef.h> /* size_t */

#define NULL_STRING     ((String){ 0 })


/* String:
 *  generic string container
 */
typedef struct String
{   char   *chars;
    size_t size,
           len;
} String;


/* Identifier:
 *  A variable or function name (just a string)
 */
typedef String Identifier;



String mkstring (char const *const str);
String stringdup (String str);
void free_string (String s);
int stringcmp (String a, String b);


#endif

