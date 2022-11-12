#include "telnet_dbg_priv.h"

static struct telnet_dbg_connection_t* new_connection(int socket) {
  struct telnet_dbg_connection_t* connection = malloc(sizeof(*connection));
  if (!connection) {
    fprintf(stderr, "ERROR> telnet_dbg_connection_t malloc\n");
    return NULL;
  }
  connection->terminated = 0;
  connection->socket = socket;
  connection->next = NULL;

  return connection;
}

struct telnet_dbg_connection_t* telnet_dbg_connection_add(
    struct telnet_dbg_connections_list_t* list,
    int socket) {
  struct telnet_dbg_connection_t* entry = new_connection(socket);
  if (!entry) {
    fprintf(stderr, "ERROR> telnet_dbg_connections_add new_connection\n");
    return NULL;
  }
  sem_wait(&list->guard);
  if (!list->root) {
    list->root = entry;
  } else {
    list->last->next = entry;
  }
  list->last = entry;

  sem_post(&list->guard);

  return entry;
}

void telnet_dbg_connection_del(struct telnet_dbg_connections_list_t* list,
                               struct telnet_dbg_connection_t* entry) {
  sem_wait(&list->guard);

  if ((list->root == entry) && (list->last == entry)) {
    list->root = NULL;
    list->last = NULL;
    goto free_entry;
  }

  if (list->root == entry) {
    list->root = list->root->next;
    goto free_entry;
  }

  struct telnet_dbg_connection_t* curr = list->root->next;
  struct telnet_dbg_connection_t* prev = list->root;
  while (curr) {
    if (entry == curr) {
      prev->next = curr->next;
      if (list->last == curr) {
        list->last = prev;
      }

      goto free_entry;
    }
    prev = curr;
    curr = curr->next;
  }

free_entry:
  close(entry->socket);
  free(entry);

  sem_post(&list->guard);
}

int telnet_dbg_connections_init(struct telnet_dbg_connections_list_t* list) {
  list->last = NULL;
  list->root = NULL;
  int res = sem_init(&list->guard, 1, 1);
  if (res != 0) {
    perror("ERROR> telnet_dbg_connections_init sem_init");
    return -1;
  }

  return 0;
}

void telnet_dbg_connections_free(struct telnet_dbg_connections_list_t* list) {
  sem_wait(&list->guard);

  struct telnet_dbg_connection_t* curr = list->root;
  while (curr) {
    struct telnet_dbg_connection_t* next = curr->next;

    curr->terminated = 1;
    pthread_cancel(curr->thread);
    pthread_join(curr->thread, NULL);
    close(curr->socket);
    free(curr);

    curr = next;
  }

  sem_post(&list->guard);

  sem_destroy(&list->guard);
}
