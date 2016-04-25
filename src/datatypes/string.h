#ifndef STRING_H
#define STRING_H

// string.h
// Wrapped unicode strings using libunistring.
// TODO: Replace/keep? most uses of char*

#include <stdarg.h>

#include "boilerplate.h"

/**********************
 * Types & Structures *
 **********************/

// Strings hold pointers to character arrays but also know how long they are.
struct string_s;
typedef struct string_s string;

/**********
 * Macros *
 **********/

// These macros define constant and static strings
#define CSTR(Name, Chars, Len) \
  string const _ ## Name ## _base = { \
    .length=Len, \
    .bytes=(uint8_t*)Chars \
  }; \
  string const * const Name = &_ ## Name ## _base

#define SSTR(Name, Chars, Len) \
  static string const _ ## Name ##_base = { \
    .length=Len, \
    .bytes=(uint8_t*)Chars \
  }; \
  static string const * const Name = &_ ## Name ## _base

/***********
 * Globals *
 ***********/

extern char* S_LOCALE;
extern char const * S_LCHARSET;

/*************************
 * Structure Definitions *
 *************************/

struct string_s {
  size_t length; // raw number of bytes without null terminator
  uint8_t* bytes;
};

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

string* copy_string(string const * const base);

string* create_string_from_ntchars(char const * const chars);
  // chars must be in local encoding & null-terminated

string* create_string_from_chars(char const * const nchars, size_t len);
  // nchars should be in local encoding but can have NULs in it

string* create_raw_string(uint8_t const * const raw, size_t len);
  // raw should be in utf-8 and is used without decoding

CLEANUP_DECL(string);

/*************
 * Functions *
 *************/

// Returns 1 if the given string contains a NUL byte, and 0 otherwise.
int s_contains_nul(string const * const s);

// Returns the length of the string in characters (minus the terminating NUL).
size_t s_get_length(string const * const s);

// Checks whether the string s exactly and completely matches the given
// characters. A number of characters less than or equal to the length of the
// string will be inspected. Returns 1 if the string matches and 0 otherwise.
// Note that in the matching case, the terminal NUL of c will not be checked,
// as the length of s will not include an extra character for NUL.
int s_check_bytes(string const * const s, char const * const c);

// Encodes the given string into the user's locale, returning a newly malloc'd
// char* and its length in the return parameter. Note that the result might
// contain NULs, and won't usually be NUL-terminated.
char* s_encode(string const * const s, size_t* rlen);

// Like s_encode, but returns a NUL-terminated string. Remember that the result
// is malloc'd and so should be freed by the caller.
char* s_encode_nt(string const * const s);

// Returns a pointer to the raw string data in UTF-8. The pointer isn't
// malloc'd, so it may be broken if other string operations are used. Note that
// the raw data may contain NULs, and should always contain a NUL at the end.
// The string's length is the total number of bytes without the final NUL.
char const * const s_raw(string const * const s);

// Returns 1 if the two strings are the same, or 0 otherwise.
int s_equals(string const * const s, string const * const other);

// Reallocates the base string to accommodate the addition of the given
// extension.
void s_append(string* base, string const * const extension);

// Creates a new string that contains the first string followed by the second.
string* s_concat(string const * const first, string const * const second);

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
