#include "server.h"
#include <arpa/inet.h>
#include <errno.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main() {
  int sockfd = -1;
  struct sockaddr_un un_addr;

  memset(&un_addr, 0, sizeof un_addr);
  un_addr.sun_family = AF_UNIX;
  strncpy(un_addr.sun_path, SOCK_PATH, sizeof(un_addr.sun_path));

  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("client: socket");
    exit(EXIT_FAILURE);
  }

  if (connect(sockfd, (struct sockaddr *)&un_addr, sizeof un_addr) != 0) {
    perror("client: connect");
    exit(EXIT_FAILURE);
  }

  // initscr();
  // int chat_height = LINES - 3;
  // int chat_width = COLS;
  //
  // int input_height = 3;
  // int input_width = COLS;
  //
  // WINDOW *chat = newwin(chat_height, chat_width, 0, 0);
  // WINDOW *input = newwin(input_height, input_width, LINES - 3, 0);
  //
  // box(chat, 0, 0);
  // box(input, 0, 0);
  //
  // wmove(input, 1, 2);
  //
  // wrefresh(chat);
  // wrefresh(input);

  while (1) {
    struct pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    int ready = poll(fds, 2, -1);
    if (ready > 0) {
      if (fds[0].revents & POLLIN) {
        char buf[MAX_MSG_LEN];
        if (fgets(buf, sizeof buf, stdin) == NULL) {
          perror("client: fgets from stdin");
        } else {
          ssize_t bytes_sent = send(sockfd, buf, strlen(buf), 0);
          if (bytes_sent <= 0)
            perror("client: send");
        }
      }
      if (fds[1].revents & POLLIN) {
        char buf[MAX_SEND_LEN];
        ssize_t bytes_recv = recv(sockfd, buf, MAX_SEND_LEN, 0);
        if (bytes_recv < 0) {
          perror("client: recv from server");
          continue;
        } else if (bytes_recv == 0) {
          break;
        }
        buf[bytes_recv] = '\0';

        if (strcmp(buf, OPT_MSG_EXIT) == 0) {
          fprintf(stdout, OPT_MSG_EXIT);
          break;
        } else {
          fprintf(stdout, "%s", buf);
        }
      }
    }
  }

  close(sockfd);
  fprintf(stdout, "Connection terminated, press any key to exit");
  getchar();
  exit(EXIT_SUCCESS);
}
