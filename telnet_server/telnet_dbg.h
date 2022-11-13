#ifndef TELNET_DBG_H
#define TELNET_DBG_H

struct telnet_dbg_t* telnet_dbg_init(const char* name_addr, int port);
int telnet_dbg_run(struct telnet_dbg_t* dbg);
int telnet_dbg_stop(struct telnet_dbg_t* dbg);
int telnet_dbg_free(struct telnet_dbg_t* dbg);

#endif  // TELNET_DBG_H
