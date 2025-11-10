#include <poll.h>

#define PORT "8080"
#define BUFSIZE 2056
#define BACKLOG 5
#define N_THREADS 8
#define MSG_BUFSIZE 5120
#define MAX_NAME_LEN 32
#define MAX_CLIENTS 50
#define SOCK_PATH "/tmp/chat.sock"
#define WELCOME_MSG "Welcome! You are now connected to chat"

typedef struct {
  int fd;
  int active;
  char name[MAX_NAME_LEN];
} client_t;

typedef struct {
  struct pollfd *fds;
  client_t *clients;
  int count;
} client_list_t;
