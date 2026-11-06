#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// String_Slice represents a view on a C String.
// This helper type makes it easier to operate on C Strings like a modern
// programming language. The internal representtion wraps around a C String with
// a measure of the length of the string.
typedef struct {
  const char *data;
  size_t count;
} String_Slice;

void ss_print(String_Slice ss) {
  for (int i = 0; i < ss.count; i++) {
    putchar(ss.data[i]);
  }
}

void ss_println(String_Slice ss) {
  ss_print(ss);
  putchar('\n');
}

// Construct a slice on the C String ignoring the NUL terminator.
//
// Ex: Hello, World0
//     ^          ^
//     data
//     count = 12 (excluding the NUL char)
String_Slice new_string_slice(const char *cstr) {
  return (String_Slice){
      .data = cstr,
      .count = strlen(cstr),
  };
}

// Convert a String_Slice into a C String.
// We do not know whether the underlying string ends with our string slice.
// Create a copy of the string in the current string slice.
// TODO: Is the caller responsible for memory management of the string?
char *to_cstring(String_Slice ss) {
  char *s = strndup(ss.data, ss.count);
  return s;
}

// Checks both the string slices for equality character by character.
bool ss_eq(String_Slice l, String_Slice r) {
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

// Chops n characters from the end of the string.
void ss_chop_right(String_Slice *s, size_t n) {
  // Clamp the characters to the be chopped to the length of the string.
  if (n > s->count)
    n = s->count;
  // No mutation of the underlying string.
  // Just decrement the length measure.
  s->count -= n;
}

// Chop 1 character from the beginning of the string.
void ss_chop_left(String_Slice *s, size_t n) {
  // Clamp the characters to the be chopped to the length of the string.
  if (n > s->count)
    n = s->count;
  // Slide the window to the right by 1 character.
  // Again, no mutation needed on the underlying string.
  s->data += n;
  s->count -= n;
}

// Trims whitespace characters from the left of the string slice window.
void ss_trim_left(String_Slice *ss) {
  while (ss->count > 0 && isspace(ss->data[0])) {
    ss_chop_left(ss, 1);
  }
}

// Trims whitespace characters from the right of the string slice window.
void ss_trim_right(String_Slice *ss) {
  while (ss->count > 0 && isspace(ss->data[ss->count - 1])) {
    ss_chop_right(ss, 1);
  }
}

// Trim whitespace characters from both the ends of the string slice window.
void ss_trim(String_Slice *ss) {
  ss_trim_left(ss);
  ss_trim_right(ss);
}

// Splits the String Slice by the first occurence of the delimiter.
//
// Ex: Hello, World ','
String_Slice ss_sep(String_Slice *ss, char delim) {
  size_t i = 0;
  while (i < ss->count && ss->data[i] != delim) {
    i += 1;
  }

  if (i < ss->count) {
    String_Slice result = {
        .data = ss->data,
        .count = i,
    };

    ss_chop_left(ss, i + 1);
    return result;
  }

  String_Slice result = *ss;
  ss_chop_left(ss, ss->count);
  return result;
}

// Splits the String Slice by the provided type.
// C provides few functions like isalphanum, isspace that we can support.
//
// Ex: Hello, World isspace
String_Slice ss_sep_by_type(String_Slice *ss, int (*istype)(int c)) {
  size_t i = 0;
  while (i < ss->count && !istype(ss->data[i])) {
    i += 1;
  }

  if (i < ss->count) {
    String_Slice result = {
        .data = ss->data,
        .count = i,
    };

    ss_chop_left(ss, i + 1);
    return result;
  }

  String_Slice result = *ss;
  ss_chop_left(ss, ss->count);
  return result;
}

// Returns the index of the element in the array if it exists.
// Else, returns -1.
int ss_arr_contains(String_Slice arr[], int len, String_Slice s) {
  // Compute the length of the array.
  for (int i = 0; i < len; i++) {
    if (ss_eq(arr[i], s)) {
      return i;
    }
  }
  return -1;
}
