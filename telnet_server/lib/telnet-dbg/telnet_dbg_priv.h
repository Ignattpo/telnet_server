#ifndef TELNET_DBG_PRIV_H
#define TELNET_DBG_PRIV_H
#include "telnet_dbg.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct telnet_dbg_connection_t {
  int socket;
  char terminated;
  pthread_t thread;
  struct telnet_dbg_connection_t* next;
};

struct telnet_dbg_connections_list_t {
  sem_t guard;
  struct telnet_dbg_connection_t* root;
  struct telnet_dbg_connection_t* last;
};

struct telnet_dbg_connection_data_t {
  struct telnet_dbg_connection_t* connection;
  struct telnet_dbg_connections_list_t* connections_list;
};

struct telnet_dbg_t {
  char* name_addr;
  int port;
  int socket;
  char terminated;
  pthread_t thread;
  struct telnet_dbg_connections_list_t connections;
};

struct telnet_dbg_connection_t* telnet_dbg_connection_add(
    struct telnet_dbg_connections_list_t* list,
    int socket);
void telnet_dbg_connection_del(struct telnet_dbg_connections_list_t* list,
                               struct telnet_dbg_connection_t* entry);
int telnet_dbg_connections_init(struct telnet_dbg_connections_list_t* list);
void telnet_dbg_connections_free(struct telnet_dbg_connections_list_t* list);

#endif  // TELNET_DBG_PRIV_H
