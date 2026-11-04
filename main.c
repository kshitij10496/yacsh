#include "strings.c"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Prompt engineering.
  while (1) {
    // Buffer to hold user input.
    // TODO: Alternate ways to not hardcode a max length. Read till newline.
    char buf[1024];

    printf("$ ");

    // Read user input from standard input stream.
    // TODO: Error handling.
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      // EOF or Ctrl+D should terminate the shell.
      printf("\n");
      break;
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
    if (ss_eq(args[0], new_string_slice("exit")) == 1)
      exit(0);

    // Fork and exec.
    pid_t pid = fork();

    switch (pid) {
    case 0: {
      // Child process
      // TODO: Support output redirection.
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
  }
  return 0;
}
