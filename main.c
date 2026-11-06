#include "strings.c"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Read a line of arbitrary size from a file until newline or EOF character.
//
// Returns a pointer to the line.
// Returns NULL on error.
//
// The caller is responsible to free() this pointer when done.
//
// NOTE: Strips any newline or EOF character.
char *readline(FILE *fp) {
  int offset = 0;
  int bufsize = 4; // Size of the initial buffer.

  // Allocate initial buffer.
  char *buf = malloc(bufsize);
  if (buf == NULL) {
    return NULL;
  }

  // Read one character at a time.
  int c = 0;
  // From man pages:
  // fgetc() return the next requested object from the stream.
  // Character values are returned as an unsigned char converted to an int.
  // If the stream is at end-of-file or a read error occurs, fgetc return EOF.
  while (c = fgetc(fp), c != '\n') {
    // Discard existing input when EOF is parsed.
    if (c == EOF) {
      free(buf);
      return NULL;
    }
    // Grow the buffer if it does not have enough room left for the new
    // character.
    if (offset == bufsize - 1) {
      // Double the buffer.
      bufsize *= 2;
      char *newBuf = realloc(buf, bufsize);
      // Fail when we cannot allocate a new buffer.
      if (newBuf == NULL) {
        free(buf);
        return NULL;
      }

      // Successful reallocation.
      buf = newBuf;
    }

    // Add the byte to the buffer.
    buf[offset] = c;
    offset += 1;
  }

  // Shrink the buffer to fit.
  // NOTE: This is a best effort, and possible to fail silently.
  if (offset < bufsize - 1) {
    char *new_buf = realloc(buf, offset + 1);
    if (new_buf != NULL) {
      buf = new_buf;
    }
  }

  // Add the NUL string terminator.
  buf[offset] = 0;
  return buf;
}

int main(int argc, char *argv[]) {
  FILE *fd_input = stdin;
  // Check for a script file as CLI parameter.
  bool is_interactive = true;

  // Assume the first parameter is a script file.
  if (argc > 1) {
    is_interactive = false;
    fd_input = fopen(argv[1], "r");
    if (fd_input == NULL) {
      perror("opening script");
      return 1;
    }
  }

  // Prompt engineering.
  while (1) {
    if (is_interactive) {
      printf("$ ");
    }

    // Buffer to hold user input.
    // TODO: Alternate ways to not hardcode a max length. Read till newline.
    char *buf = readline(fd_input);
    if (buf == NULL) {
      // EOF or Ctrl+D should terminate the shell.
      printf("\n");
      break;
    }

    // COMMENTS
    // Ignore everything after the "#" character.
    char *found_comment = strchr(buf, '#');
    if (found_comment != NULL) {
      // Set to NUL terminator to trim from the input buffer.
      *found_comment = 0;
    }

    // Build a string slice to simplify string manipulation.
    // Thus, we do not need to reallocate the input string again.
    // We work with views into the same shared string.
    // We can mutate the windows such as input whenever we desire.
    String_Slice input = new_string_slice(buf);

    // Sanitize user input - trim newline character.
    // Replace with the NUL termination character.
    String_Slice sanitized_input = ss_sep(&input, '\n');

    // TODO: How to dynamically allocate this memory?
    String_Slice args[20];
    int nargs = 0;
    while (sanitized_input.count > 0) {
      String_Slice word = ss_sep_by_type(&sanitized_input, isspace);

      // Sanitize each word and ignore empty spaces.
      ss_trim(&word);
      if (word.count == 0) {
        continue;
      }
      args[nargs] = word;
      nargs++;
    }

    // Fail fast when the user provides no input.
    if (nargs == 0)
      continue;

    // "exit" builtin command to exit the shell.
    if (ss_eq(args[0], new_string_slice("exit")))
      exit(0);

    // Fork and exec.
    pid_t pid = fork();

    switch (pid) {
    case 0: {
      // Child process
      // Output Redirection.
      int idx = ss_arr_contains(args, nargs, new_string_slice(">"));
      if (idx >= 0) {
        // The next argument should be the output stream of the child process.
        // TODO: Support non-space separated argument like ">result".
        String_Slice outputStream = args[idx + 1];
        // Change the output stream from stdout to the redirected stream.
        int fdOutput =
            open(to_cstring(outputStream), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dup2(fdOutput, 1);
        // dup handles the writes. Safe to close the FD.
        close(fdOutput);

        // Trim the list of args to before the Redirection operator.
        nargs = idx;
      }

      // Input Redirection
      // NOTE: We assume Input Redirection can only potentially occur before any
      // Output Redirection.
      int idxInput = ss_arr_contains(args, nargs, new_string_slice("<"));
      if (idxInput >= 0) {
        // The next argument should be the input stream of the child process.
        // TODO: Support non-space separated argument like ">result".
        String_Slice inputStream = args[idxInput + 1];
        // Change the input stream from stdin to the redirected stream.
        int fdInput =
            open(to_cstring(inputStream), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dup2(fdInput, 0);
        // dup handles the writes. Safe to close the FD.
        close(fdInput);

        // Trim the list of args to before the Redirection operator.
        nargs = idxInput;
      }

      // Convert the user input to args.
      char *execArgs[20];
      for (int i = 0; i < nargs; i++) {
        execArgs[i] = to_cstring(args[i]);
      }
      execArgs[nargs] = NULL;

      execvp(execArgs[0], execArgs);
      // Failed for some reason so exit the shell.
      perror(execArgs[0]);
      exit(1);
    }
    case -1:
      // Failed to fork the parent process.
      // Thus, no child process is created.
      // Parent process should handle this.
      // errono is set to indicate the error.
      perror("fork");
      break;

    default:
      // Parent process.
      //
      // Wait for the child to finish.
      // TODO: Check for the flags set.
      wait(NULL);
    }
    free(buf);
  }
  return 0;
}
