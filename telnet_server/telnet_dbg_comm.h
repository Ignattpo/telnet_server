#ifndef TELNET_DBG_COMMUNICATION_H
#define TELNET_DBG_COMMUNICATION_H
#include "telnet_dbg_priv.h"

void telnet_dbg_comm_parse(int socket, char* buff, size_t buff_size);

typedef void (*function_t)(int, int, char**);
struct commands_list_t {
  const char* name;
  function_t function;
};

#endif  // TELNET_DBG_COMMUNICATION_H
