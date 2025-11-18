// ============================================================================
// CLIENT CODE
// ============================================================================

/*
Questions to answer at top of client.c:
(You should not need to change the code in client.c)
1. What is the address of the server it is trying to connect to (IP address and
port number). Answer: The server address is 127.0.0.1 (localhost) and port 8000.
   This is defined by the ADDR macro ("127.0.0.1") and PORT macro (8000).
   The IP address is set using inet_pton() and the port is set using
htons(PORT).

2. Is it UDP or TCP? How do you know?
   Answer: It is TCP. We know this because the socket is created with
SOCK_STREAM (socket(AF_INET, SOCK_STREAM, 0)), which indicates a TCP connection.
   Additionally, the client uses connect() which is used for connection-oriented
   protocols like TCP. UDP would use SOCK_DGRAM instead.

3. The client is going to send some data to the server. Where does it get this
data from? How can you tell in the code? Answer: The client gets data from
standard input (stdin/keyboard). This is evident from the read() call that reads
from STDIN_FILENO: "while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1)".
   STDIN_FILENO is file descriptor 0, which represents standard input.

4. How does the client program end? How can you tell that in the code?
   Answer: The client program ends when the read() call from stdin returns 1 or
less. This happens when the user presses Ctrl+D (EOF) or when there's a read
error. The while loop condition "while ((num_read = read(STDIN_FILENO, buf,
BUF_SIZE)) > 1)" will exit when num_read <= 1. After exiting the loop, the
socket is closed with close(sfd) and the program exits with exit(EXIT_SUCCESS).
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 64
#define ADDR "127.0.0.1"

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main() {
  struct sockaddr_in addr;
  int sfd;
  ssize_t num_read;
  char buf[BUF_SIZE];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, ADDR, &addr.sin_addr) <= 0) {
    handle_error("inet_pton");
  }

  int res = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (res == -1) {
    handle_error("connect");
  }

  while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1) {
    if (write(sfd, buf, num_read) != num_read) {
      handle_error("write");
    }
    printf("Just sent %zd bytes.\n", num_read);
  }

  if (num_read == -1) {
    handle_error("read");
  }

  close(sfd);
  exit(EXIT_SUCCESS);
}

// ============================================================================
// SERVER CODE
// ============================================================================

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 64
#define PORT 8000
#define LISTEN_BACKLOG 32

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int total_message_count = 0;
int client_id_counter = 1;

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;

struct client_info {
  int cfd;
  int client_id;
};

void *handle_client(void *arg) {
  struct client_info *client = (struct client_info *)arg;

  int cfd = client->cfd;
  int client_id = client->client_id;

  free(client);

  char buf[BUF_SIZE];
  ssize_t num_read;

  while ((num_read = read(cfd, buf, BUF_SIZE)) > 0) {
    if (buf[num_read - 1] == '\n') {
      buf[num_read - 1] = '\0';
    } else {
      buf[num_read] = '\0';
    }

    pthread_mutex_lock(&count_mutex);
    total_message_count++;
    int current_count = total_message_count;
    pthread_mutex_unlock(&count_mutex);

    printf("Msg # %3d; Client ID %d: %s\n", current_count, client_id, buf);
  }

  close(cfd);

  printf("Ending thread for client %d\n", client_id);

  return NULL;
}

int main() {
  struct sockaddr_in addr;
  int sfd;

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }

  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    handle_error("listen");
  }

  for (;;) {
    int cfd = accept(sfd, NULL, NULL);
    if (cfd == -1) {
      handle_error("accept");
    }

    pthread_mutex_lock(&client_id_mutex);
    int current_client_id = client_id_counter;
    client_id_counter++;
    pthread_mutex_unlock(&client_id_mutex);

    printf("New client created! ID %d on socket FD %d\n", current_client_id,
           cfd);

    struct client_info *client = malloc(sizeof(struct client_info));
    if (client == NULL) {
      handle_error("malloc");
    }
    client->cfd = cfd;
    client->client_id = current_client_id;

    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_client, client) != 0) {
      handle_error("pthread_create");
    }

    if (pthread_detach(thread) != 0) {
      handle_error("pthread_detach");
    }
  }

  if (close(sfd) == -1) {
    handle_error("close");
  }

  return 0;
}
