#include "telnet_dbg_priv.h"

#include <dlfcn.h>
#include <netdb.h>
#include <netinet/in.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void* communication(void* thread_data) {
  char buf[1024];
  struct telnet_dbg_connect_t* connect = thread_data;

  while (!connect->terminated) {
    int bytes_read = recv(connect->socket, buf, sizeof(buf), 0);
    printf("recv %d %d %s\n", bytes_read, connect->socket, buf);
    for (int i = 0; i < bytes_read; i++) {
      printf("%x ", buf[i]);
    }

    if ((buf[0] == '^') && (buf[1] == ']')) {
      send(connect->socket, "BUY\n", 4, 0);
      connect->terminated = 1;
      break;
    }
    printf("\n");
    if (bytes_read <= 0) {
      connect->terminated = 1;
      break;
    }

    send(connect->socket, buf, bytes_read, 0);
  }

  close(connect->socket);

  return 0;
}

void* server_thread(void* thread_data) {
  struct telnet_dbg_t* dbg = thread_data;

  const struct timespec time = {.tv_sec = 1, .tv_nsec = 0};
  const sigset_t sigmask = {SIGINT};

  int res = listen(dbg->socket, 1);
  if (res == -1) {
    fprintf(stderr, "ERROR> telnet_dbg addres: %s port: %d \n", dbg->name_addr,
            dbg->port);
    perror("ERROR> telnet_dbg listen");
    close(dbg->socket);
    return 0;
  }
  while (!dbg->terminated) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(dbg->socket, &rfds);

    res = pselect(dbg->socket + 1, &rfds, NULL, NULL, &time, &sigmask);
    if (res == -1) {
      fprintf(stderr, "ERROR> telnet_dbg addres: %s port: %d \n",
              dbg->name_addr, dbg->port);
      perror("ERROR> telnet_dbg pselect");
      break;
    }
    if (res == 0) {
      continue;
    }

    int sock = accept(dbg->socket, NULL, NULL);
    if (sock < 0) {
      fprintf(stderr, "ERROR> telnet_dbg addres: %s port: %d \n",
              dbg->name_addr, dbg->port);
      perror("ERROR> telnet_dbg accept");
      break;
    }

    struct telnet_dbg_connect_t connect;
    connect.terminated = 0;
    connect.socket = sock;
    res = pthread_create(&connect.thread, NULL, communication, &connect);
    if (res != 0) {
      fprintf(stderr, "ERROR> telnet_dbg pthread_create addres: %s port: %d \n",
              dbg->name_addr, dbg->port);
    }
  }

  close(dbg->socket);
  return 0;
}

struct telnet_dbg_t* telnet_dbg_init(const char* name_addr, int port) {
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    goto aborting;
  }

  struct hostent* hosten = gethostbyname(name_addr);
  if (!hosten) {
    perror("telnet_dbg_init gethostbyname");
    return NULL;
  }

  struct in_addr** addr_list = (struct in_addr**)hosten->h_addr_list;
  if (addr_list[0] == NULL) {
    fprintf(stderr, "telnet_dbg_init incorrect addres %s\n", name_addr);
    return NULL;
  }

  struct sockaddr_in sock_addr;
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons(port);
  sock_addr.sin_addr = *addr_list[0];

  int res =
      bind(server_socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
  if (res < 0) {
    perror("telnet_dbg_init bind");
    goto aborting;
  }
  struct telnet_dbg_t* dbg = malloc(sizeof(*dbg));
  if (!dbg) {
    fprintf(stderr, "telnet_dbg_init malloc %s\n", name_addr);
    goto aborting;
  }

  dbg->name_addr = strdup(name_addr);
  dbg->port = port;
  dbg->socket = server_socket;
  dbg->terminated = 0;

  return dbg;

aborting:

  close(server_socket);
  return NULL;
}

int telnet_dbg_run(struct telnet_dbg_t* dbg) {
  int res = pthread_create(&dbg->thread, NULL, server_thread, dbg);
  if (res != 0) {
    fprintf(stderr,
            "ERROR> telnet_dbg_run pthread_create addres: %s port: %d \n",
            dbg->name_addr, dbg->port);
  }

  return 0;
}

int telnet_dbg_stop(struct telnet_dbg_t* dbg) {
  dbg->terminated = 1;
  pthread_join(dbg->thread, NULL);
  return 0;
}

int telnet_dbg_free(struct telnet_dbg_t* dbg) {
  free(dbg->name_addr);
  return 0;
}
