#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {

  char line[2000];

  printf("Enter programs to run\n");

  while (true) {

    printf("> ");
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin)) {
      putchar('\n');
      break;
    }

    char *new_line = strchr(line, '\n');
    if (new_line)
      *new_line = '\0';
    if (line[0] == '\0')
      continue;

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      continue;
    }

    if (pid == 0) {
      execl(line, line, (char *)NULL);
      puts("Exec failure");
      _exit(127);

    } else {
      waitpid(pid, NULL, 0);
    }
  }

  return 0;
}
