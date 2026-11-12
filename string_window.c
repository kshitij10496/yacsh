#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// String_View represents a view on a C String.
// This helper type makes it easier to operate on C Strings like a modern
// programming language. The internal representation wraps around a C String
// with a measure of the length of the string.
typedef struct {
  const char *data;
  size_t count;
} String_View;

void sv_print(String_View sv, FILE *stream) {
  for (int i = 0; i < sv.count; i++) {
    fputc(sv.data[i], stream);
  }
}

void sv_println(String_View sv, FILE *stream) {
  sv_print(sv, stream);
  fputc('\n', stream);
}

// Construct a view on the C String ignoring the NUL terminator.
//
// Ex: Hello, World0
//     ^          ^
//     data
//     count = 12 (excluding the NUL char)
String_View new_string_view(const char *cstr) {
  return (String_View){
      .data = cstr,
      .count = strlen(cstr),
  };
}

// Convert a String_View into a C String.
//
// NOTE: We do not know whether the underlying string ends with our string view.
// Thus, we cannot simply add a NUL character to the end of the string view.
// Consequently, we create a copy of the string defined in the current string
// view. The caller is responsible for memory management of the string.
char *to_cstring(String_View ss) {
  char *s = strndup(ss.data, ss.count);
  return s;
}

// Two string views are considered "equal" if both of them hold the same
// character values.
bool sv_eq(String_View l, String_View r) {
  // Fail fast.
  if (l.count != r.count) {
    return false;
  }

  for (int i = 0; i < l.count; i++) {
    if (l.data[i] != r.data[i]) {
      return false;
    }
  }

  return true;
}

bool sv_begins_with(String_View sv, char c) {
  return ((sv.count > 0) && (sv.data[0] == c));
}

// Chops n characters from the end of the string view.
// Clamps the characters to be chopped to the length of the string.
//
// NOTE: The underlying string buffer is not mutated by this operation.
// Only the string view is clipped from the end.
void sv_chop_right(String_View *sv, size_t n) {
  if (n > sv->count) {
    n = sv->count;
  }

  // Slide the right end of the window by n characters.
  sv->count -= n;
}

// Chops n character from the beginning of the string view.
// Clamps the characters to be chopped to the length of the string.
//
// NOTE: The underlying string buffer is not mutated by this operation.
// Only the string view is clipped from the beginning.
void sv_chop_left(String_View *sv, size_t n) {
  if (n > sv->count) {
    n = sv->count;
  }
  // Slide the left end of the window by n characters.
  sv->data += n;
  sv->count -= n;
}

// Trims whitespace characters from the beginning of the string window.
//
// NOTE: Mutates the string window.
void sv_trim_left(String_View *sv) {
  while (sv->count > 0 && isspace(sv->data[0])) {
    // Mutate the window 1 character at a time.
    // TODO: Would a single bulk mutation perform better?
    sv_chop_left(sv, 1);
  }
}

// Trims whitespace characters from the end of the string window.
//
// NOTE: Mutates the string window.
void sv_trim_right(String_View *sv) {
  while (sv->count > 0 && isspace(sv->data[sv->count - 1])) {
    // Mutate the window 1 character at a time.
    // TODO: Would a single bulk mutation perform better?
    sv_chop_right(sv, 1);
  }
}

// Trim whitespace characters from both ends of the string window.
//
// NOTE: Mutates the string window.
void sv_trim(String_View *ss) {
  sv_trim_left(ss);
  sv_trim_right(ss);
}

// Splits the string window by the first occurence of the delimiter.
// Returns a new string window with the substring before the delimiter.
// NOTE:Mutates the input string view to a substring after stripping the
// delimiter.
//
// Ex: ("key=value", '=')
// Returned value: String_View("key")
// Input: "value"
String_View sv_sep(String_View *ss, char delim) {
  size_t i = 0;
  while (i < ss->count && ss->data[i] != delim) {
    i += 1;
  }

  if (i < ss->count) {
    String_View result = {
        .data = ss->data,
        .count = i,
    };

    sv_chop_left(ss, i + 1);
    return result;
  }

  String_View result = *ss;
  sv_chop_left(ss, ss->count);
  return result;
}

// Splits the string window by the provided function.
// The Standard C Library (libc, glibc, musl) ship with functions like
// isalphanum, isspace. NOTE:Mutates the input string view to a substring after
// stripping the delimiter.
//
// Ex: ("Hello, World", isspace)
// Returns: "Hello,"
// Input: "World"
String_View sv_sep_by_type(String_View *sv, int (*istype)(int c)) {
  size_t i = 0;
  while (i < sv->count && !istype(sv->data[i])) {
    i += 1;
  }

  if (i < sv->count) {
    String_View result = {
        .data = sv->data,
        .count = i,
    };

    sv_chop_left(sv, i + 1);
    return result;
  }

  String_View result = *sv;
  sv_chop_left(sv, sv->count);
  return result;
}

// Returns the index of the element in the array if it exists.
// Else, returns -1.
int sv_arr_contains(String_View arr[], int len, String_View s) {
  // Compute the length of the array.
  for (int i = 0; i < len; i++) {
    if (sv_eq(arr[i], s)) {
      return i;
    }
  }
  return -1;
}
