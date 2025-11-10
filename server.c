// Copyright (c) 2025 Matt Oliveira. All Rights Reserved.
#include "./server.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

client_list_t client_list;

int main(int argc, char *argv[]) {
  int serv_fd;
  struct sockaddr_un serv_addr, client_addr;
  socklen_t client_len;

  serv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (serv_fd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  unlink(SOCK_PATH);
  memset(&serv_addr, 0, sizeof serv_addr);
  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCK_PATH, sizeof(serv_addr.sun_path) - 1);

  if (bind(serv_fd, (struct sockaddr *)&serv_addr, sizeof serv_addr) == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if (listen(serv_fd, BACKLOG) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "Client connected: %d\n", serv_fd);

  memset(&client_list, 0, sizeof client_list);
  client_list.clients = malloc(MAX_CLIENTS * sizeof(client_t));
  client_list.fds = malloc(MAX_CLIENTS * sizeof(struct pollfd));

  client_list.fds[0].fd = serv_fd;
  client_list.fds[0].events = POLLIN;

  while (1) {
    int serv_offset = client_list.count + 1;
    int ready = poll(client_list.fds, serv_offset, -1);

    if (ready > 0) {
      if (client_list.fds[0].revents & POLLIN) {
        int client_fd = accept(serv_fd, NULL, NULL);

        client_t client;
        client.fd = client_fd;
        client_list.clients[serv_offset] = client;
        client_list.fds[serv_offset].fd = client_fd;
        client_list.fds[serv_offset].events = POLLIN;
        client_list.count++;
      }

      for (int i = 1; i < serv_offset; i++) {
        if (client_list.fds[i].revents & POLLIN) {
          char buf[MSG_BUFSIZE];
          ssize_t bytes_received;
          bytes_received = recv(client_list.fds[i].fd, buf, sizeof buf, 0);

          if (bytes_received > 0) {
            ssize_t bytes_sent;
            for (int j = 1; j < serv_offset; j++) {
              if (client_list.fds[j].fd == client_list.fds[i].fd)
                continue;
              bytes_sent =
                  send(client_list.clients[j].fd, buf, bytes_received, 0);
            }
          }
        }
      }

      for (int k = 1; k < serv_offset; k++) {
        if (client_list.fds[k].revents & POLLHUP) {
          close(client_list.fds[k].fd);
          if (k != client_list.count) {
            client_list.fds[k] = client_list.fds[client_list.count];
            client_list.clients[k] = client_list.clients[client_list.count];
          }
          client_list.count--;
          k--;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}
