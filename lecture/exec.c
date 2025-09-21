#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main() {
  printf("Hello\n");
  pid_t pid = fork();

  if (pid == 0) {
    printf("Child\n");
  } else {
    printf("Parent\n");
    execl("/bin/ls", "/bin/l", "-a", NULL);
  }

  printf("World\n");
}
