#include "strings.c"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  while (1) {
    // Buffer to hold user input.
    // TODO: Alternate ways to not hardcode a max length. Read till newline.
    char buf[1024];

    // Prompt engineering.
    printf("$ ");

    // Read user input from standard input stream.
    // TODO: Error handling.
    if (fgets(buf, sizeof(buf), stdin) == NULL)
      continue;

    // Build a string slice to simplify string manipulation.
    String_Slice input = new_string_slice(buf);

    // Sanitize user input - trim newline character.
    // Replace with the NUL termination character.
    String_Slice sanitized_input = ss_sep(&input, '\n');

    char *args[20];
    int nargs = 0;
    while (sanitized_input.count > 0) {
      String_Slice word = ss_sep_by_type(&sanitized_input, isspace);
      // Sanitize each word and ignore empty spaces.
      ss_trim(&word);
      if (word.count == 0) {
        continue;
      }
      // Clone the string slice into program's dynamic memory.
      args[nargs] = to_cstring(word);
      nargs++;
    }
    // Hack needed for the syscall.
    args[nargs] = NULL;

    // Fail fast when the user provides no input.
    if (args[0] == NULL)
      continue;

    // "exit" builtin command to exit the shell.
    if (strcmp(args[0], "exit") == 0)
      exit(0);

    // Fork and exec.
    pid_t pid = fork();

    switch (pid) {
    case 0:
      // Child process
      // TODO: Support output redirection.
      execvp(args[0], args);
      // Something went wrong.
      fprintf(stderr, "failed to exec: %s\n", buf);
      break;
    case -1:
      // Failed to fork the parent process.
      // Thus, no child process is created.
      // Parent process should handle this.
      // errono is set to indicate the error.
      break;

    default:
      // Parent process.
      //
      // Wait for the child to finish.
      // TODO: Check for the flags set.
      wait(NULL);
    }
  }
  return 0;
}
