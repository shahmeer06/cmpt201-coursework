#include <stdio.h>
#include <unistd.h>

int main() {
  fork();
  for (int i = 0; i < 5; i++) {

    printf("Sleep\n");
    sleep(3);
  }
}
