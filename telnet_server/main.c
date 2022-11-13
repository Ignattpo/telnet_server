#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include "telnet-dbg/telnet_dbg.h"

int main() {
  struct telnet_dbg_t* dbg = telnet_dbg_init("localhost", 2323);

  telnet_dbg_run(dbg);
  sleep(60);

  telnet_dbg_stop(dbg);

  telnet_dbg_free(dbg);

  return 0;
}
