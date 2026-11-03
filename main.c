#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  while (1) {
    // Buffer to hold user input.
    // TODO: Explore using a StringSlice type for easily operating on strings.
    // character.
    // TODO: Alternate ways to not hardcode a max length. Read till newline
    char buf[1024];

    // Prompt engineering.
    printf("$ ");

    // Read user input from standard input stream.
    // TODO: Error handling.
    if (fgets(buf, sizeof(buf), stdin) == NULL)
      continue;

    // Sanitize user input - trim newline character.
    // Replace with the NUL termination character.
    char *nl = strchr(buf, '\n');
    if (nl)
      *nl = 0;

    // Split buffer into args by whitespace character.
    // TODO: Find a better way to dynamically allow multiple words.
    char *args[20];
    int nargs = 0;

    // TODO: Trim whitespace characters from each word.
    // TODO: Can we use a general whitespace character instead of hardcoding?
    char *cursor = buf; // Movable view into the buffer string.
    char *token;
    while ((token = strsep(&cursor, " \t")) != NULL) {
      args[nargs] = token;
      nargs++;
    }
    // Add a NULL pointer as the last element of the array of args.
    args[nargs] = NULL;

    if (args[0] == NULL)
      continue;

    // "exit" command to exit the shell.
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
