#include <_stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  while (1) {
    // Buffer to hold user input.
    // TODO: Alternate ways to not hardcode a max length. Read till newline
    // character.
    char buf[1024];

    // Prompt engineering.
    printf("$ ");

    // Read user input from standard input stream.
    // TODO: Error handling.
    fgets(buf, 1024, stdin);

    // Sanitize user input - trim newline character.
    // Replace with the NUL termination character.
    char *nl = strchr(buf, '\n');
    if (nl)
      *nl = 0;

    // Split buffer into args by whitespace character.
    char *args[20];
    int nargs = 0;

    args[nargs] = strtok(buf, " ");
    while (args[nargs] != NULL) {
      args[++nargs] = strtok(NULL, " ");
    }

    if (args[0] == NULL) continue;

    // "exit" command to exit the shell.
    if (strcmp(args[0], "exit") == 0) exit(0);

    // Fork and exec.
    pid_t pid = fork();

    switch (pid) {
    case 0:
      // Child process
      // TODO: Support output redirection.
      execvp(args[0], args);
      // Something went wrong.
      fprintf(stderr, "failed to exec %s\n", buf);

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
