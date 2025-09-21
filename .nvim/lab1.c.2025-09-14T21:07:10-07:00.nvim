#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

  char *line = NULL;
  size_t len = 0;

  printf("Enter some text: ");
  getline(&line, &len, stdin);

  size_t line_len = strlen(line);
  if (line_len > 0 && line[line_len - 1] == '\n') {
    line[line_len - 1] = '\0';
  }

  printf("Tokens:\n");

  char *saveptr;
  char *token = strtok_r(line, " ", &saveptr);
  while (token != NULL) {
    printf(" %s\n", token);
    token = strtok_r(NULL, " ", &saveptr);
  }

  free(line);
  return 0;
}
