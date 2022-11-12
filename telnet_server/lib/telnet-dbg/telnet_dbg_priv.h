#ifndef TELNET_DBG_PRIV_H
#define TELNET_DBG_PRIV_H
#include "telnet_dbg.h"

#include <pthread.h>
#include <semaphore.h>

struct telnet_dbg_connect_t {
  int socket;
  char terminated;
  pthread_t thread;
};

struct telnet_dbg_connects_t {
  struct telnet_dbg_client_t* client;
  struct telnet_dbg_client_t* next;
};

struct telnet_dbg_connects_list_t {
  sem_t guard;
  struct telnet_dbg_connects_t* root;
};

struct telnet_dbg_t {
  char* name_addr;
  int port;
  int socket;
  char terminated;
  pthread_t thread;
  struct telnet_dbg_connects_list_t clients;
};

#endif  // TELNET_DBG_PRIV_H
