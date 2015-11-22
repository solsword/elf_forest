#ifndef STRING_H
#define STRING_H

// string.h
// Wrapped unicode strings using libunistring.
// TODO: This datatype!
// TODO: Replace most uses of char*

#include <stdarg.h>

/**********************
 * Types & Structures *
 **********************/

// Strings hold pointers to character arrays but also know how long they are.
// Note that string_s is declared in string.c: other files shouldn't mess with
// its internals.
struct string_s;
typedef struct string_s string;

/***********
 * Globals *
 ***********/

extern char* S_LOCALE;
extern char const * S_LCHARSET;

/********
 * Init *
 ********/

// Must be called so that the strings system is aware of the user's locale &
// the corresponding charset.
void init_strings(void);

/******************************
 * Constructors & Destructors *
 ******************************/

string* create_string(void);

string* copy_string(string *base);

string* create_string_from_ntchars(char const * const chars);
  // chars must be in local encoding & null-terminated

string* create_string_from_chars(char const * const nchars, size_t len);
  // nchars should be in local encoding but can have NULs in it.

void cleanup_string(string* s);

/*************
 * Functions *
 *************/

// Returns 1 if the given string contains a NUL byte, and 0 otherwise.
size_t s_contains_nul(string* s);

// Encodes the given string into the user's locale, returning a newly malloc'd
// char* and its length in the return parameter. Note that the result might
// contain NULs, and won't necessarily be NUL-terminated.
char* s_encode(string* s, size_t* rlen);

// Like s_encode, but returns a NUL-terminated string. Remember that the result
// is malloc'd and so should be freed by the caller.
char* s_encode_nt(string* s);

// Reallocates the base string to accommodate the addition of the given
// extension.
void s_append(string* base, string const * const extension);

// Creates a new string that contains the first string followed by the second.
string* s_concat(string* first, string* second);

// Joins together several strings, separating them with the given separator.
// The list of arguments to join MUST be terminated with a NULL.
string* s_join(string const * const sep, ...);
string* s_vjoin(string const * const sep, va_list args);

// Printfs starting with a char* format string allocating and returning a new
// string pointer.
string* s_sprintf(char const * const fmt, ...);
string* s_vsprintf(char const * const fmt, va_list args);

// Prints the string to stdout (optionally adding a newline). If the string
// contains NUL bytes, printing will stop there, and in the println case, no
// newline will be added. The string is encoded into the user's encoding first.
void s_print(string *s);
void s_println(string *s);

#endif // #ifndef STRING_H
