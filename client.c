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

  initscr();
  cbreak();
  noecho();

  int chat_height = LINES - 3;
  int chat_width = COLS;

  int input_height = 3;
  int input_width = COLS;

  WINDOW *chat = newwin(chat_height, chat_width, 0, 0);
  WINDOW *input = newwin(input_height, input_width, LINES - 3, 0);
  scrollok(input, FALSE);

  WINDOW *chat_content = derwin(chat, chat_height - 2, chat_width - 3, 1, 2);
  scrollok(chat_content, TRUE);

  box(chat, 0, 0);
  box(input, 0, 0);

  keypad(input, TRUE);
  wmove(chat, 1, 2);
  wmove(input, 1, 2);

  wrefresh(chat);
  wrefresh(input);

  char input_buf[MAX_MSG_LEN];
  memset(input_buf, 0, MAX_MSG_LEN);

  int count = 0;
  int input_offset = 0;

  while (1) {
    struct pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    int ready = poll(fds, 2, -1);
    if (ready > 0) {
      if (fds[0].revents & POLLIN) {
        int ch = wgetch(input);
        int vis_width = COLS - 4;

        if (ch == '\n') {
          input_buf[count++] = '\n';
          input_buf[count] = '\0';

          wclear(input);
          box(input, 0, 0);
          wmove(input, 1, 2);
          wrefresh(input);

          if (count >= MAX_MSG_LEN) {
            wprintw(input, "\nMax message length exceeded");
            wrefresh(input);
          } else {
            ssize_t bytes_sent = send(sockfd, input_buf, count, 0);
            if (bytes_sent <= 0)
              perror("client: send");

            memset(input_buf, 0, MAX_MSG_LEN);
            count = 0;
            input_offset = 0;
          }
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
          if (count > 0) {
            count--;
            input_buf[count] = '\0';

            int vis_count = count - input_offset;
            if (vis_count < vis_width - 1 && input_offset > 0)
              input_offset--;

            wmove(input, 1, 2);
            wclrtoeol(input);

            int num_chars_displayed = (count - input_offset < vis_width)
                                          ? count - input_offset
                                          : vis_width;
            for (int i = 0; i < num_chars_displayed; i++)
              waddch(input, input_buf[input_offset + i]);

            box(input, 0, 0);
            wrefresh(input);
          }
        } else {
          int vis_count = count - input_offset;
          if (vis_count >= vis_width) {
            input_offset++;
            wmove(input, 1, 2);
            wdelch(input);
            wmove(input, 1, 2 + vis_width - 1);
          }
          input_buf[count++] = ch;
          waddch(input, ch);
          box(input, 0, 0);
          wrefresh(input);
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
          wprintw(chat_content, OPT_MSG_EXIT);
          wrefresh(chat);
          break;
        } else {
          wprintw(chat_content, "%s", buf);

          int y, x;
          getyx(chat, y, x);
          wmove(chat, y, 2);
          (void)x;

          wrefresh(chat);
          wrefresh(chat_content);
          wrefresh(input);
        }
      }
    }
  }

  close(sockfd);
  endwin();
  fprintf(stdout, "Connection terminated\n");
  exit(EXIT_SUCCESS);
}
