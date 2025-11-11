#include <poll.h>

#define BACKLOG 5
#define PORT "8080"
#define SOCK_PATH "/tmp/chat.sock"

#define DELIMITERS " \n\t\f\v"
#define MAX_CLIENTS 50
#define MAX_NAME_LEN 32
#define MAX_SEND_LEN 5164
#define MAX_MSG_LEN 5120

#define OPT_MSG_EXIT "Exiting m-chat\n"
#define OPT_MSG_MENU "/nick to change your nickname\n/exit to exit\n"
#define OPT_MSG_NICK_SUCCESS "Your nickname has been changed to:"

#define ERR_NICK_EMPTY "Nickname change failed: nickname field empty\n"
#define ERR_NICK_LEN                                                           \
  "Nickname change failed: nickname exceeds the max length [32 characters]\n"
#define ERR_OPT_NOT_FOUND "Menu option not found:"

typedef struct {
  int fd;
  char name[MAX_NAME_LEN];
} client_t;

typedef struct {
  struct pollfd *fds;
  client_t *clients;
  int count;
} client_list_t;
