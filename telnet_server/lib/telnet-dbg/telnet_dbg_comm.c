#include "telnet_dbg_comm.h"

#include <dlfcn.h>
#include <inttypes.h>

/*
Пример работы какой хотелось бы видеть
### < комментарий
telnet localhost 2323
Trying ::1...
Connected to localhost.
# Дамп памяти
> mem dump 0x2378423 64
0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // PQRSTUVW
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // XYZ[\]^_

# Пишем в память адрес, далее список байт
> mem write 0x2378423 01 02 FF AA CC
writed 5 bytes to address 0x2378423


# Запускаем функцию - так же резолвим через dlsym. Возможно переменное
количество аргументов. Аргументы в виде 32х битных чисел и NULL-terminated строк
(до 5 аргументов) >
 c malloc(0xFF)
func ‘malloc’ at 0x76cadc9 returned 0xFA273A2

>c strlen(“Helllo”)
func ‘strlen’ at 0x76caabb returned 0x6
*/

void dbg_memory_dump(int socket, int count, char** argv);
void dbg_memory_write(int socket, int count, char** argv);
void dbg_resolve(int socket, int count, char** argv);
void dbg_read(int socket, int count, char** argv);
void dbg_write(int socket, int count, char** argv);
void dbg_function(int socket, int count, char** argv);

static const int commands_count = 6;
static struct commands_list_t commands[] = {
    {.name = "mem_dump", .function = dbg_memory_dump},
    {.name = "mem_write", .function = dbg_memory_write},
    {.name = "s", .function = dbg_resolve},
    {.name = "r", .function = dbg_read},
    {.name = "w", .function = dbg_write},
    {.name = "c", .function = dbg_function}};

static int get_count_argv(char* buff, size_t buff_size) {
  int count = 1;
  for (size_t i = 0; i < buff_size; ++i) {
    if (buff[i] == ' ') {
      count++;
    }
  }
  return count;
}

static void get_argv(char* buff, int argv_count, char** argv) {
  char* p = buff;
  for (int i = 0; i < argv_count; ++i) {
    char* str = strtok_r(p, " ", &p);
    argv[i] = str;
  }
  //  Удаляем из последнего аргумента перевод строки и перенос каретки
  int len = strlen(argv[argv_count - 1]);
  argv[argv_count - 1][len - 2] = '\0';
}

void telnet_dbg_comm_parse(int socket, char* buff, size_t buff_size) {
  char buff_msg[buff_size];
  //  Копируем только текущее сообщение, для избавление от мусора
  memcpy(buff_msg, buff, buff_size);
  int argv_count = get_count_argv(buff_msg, buff_size);
  char* argv[argv_count];
  get_argv(buff_msg, argv_count, argv);
  char comand_found = 0;
  for (int i = 0; i < commands_count; ++i) {
    if (!strcmp(commands[i].name, argv[0])) {
      commands[i].function(socket, argv_count, argv);
      comand_found = 1;
    }
  }
  if (!comand_found) {
    char err[255];
    sprintf(err, "%s command not found\n", argv[0]);
    send(socket, err, strlen(err), 0);
  }
}

static char is_number(char* argv) {
  if ((argv[0] >= '0') && (argv[0] <= '9')) {
    return 1;
  }
  return 0;
}

static long get_number(char* argv) {
  int base = 10;
  if ((argv[0] == '0') && (argv[1] == 'x')) {
    base = 16;
  }

  return strtoul(argv, NULL, base);
}

static void* get_pointer(char* argv) {
  if (is_number(argv)) {
    long addr = get_number(argv);
    return (void*)addr;
  } else {
    dlerror();
    void* ptr = dlsym(NULL, argv);
    void* error = dlerror();
    if (!error) {
      return ptr;
    } else {
      return NULL;
    }
  }
  return NULL;
}

void dbg_memory_dump(int socket, int count, char** argv) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
  if (count != 3) {
    char* err = "mem_dump command has an incorrect number of arguments\n";
    send(socket, err, strlen(err), 0);
    return;
  }

  void* ptr = NULL;
  ptr = get_pointer(argv[1]);
  if (!ptr) {
    char err[255];
    sprintf(err, "Address '%s' is NULL\n", argv[1]);
    send(socket, err, strlen(err), 0);
    return;
  }
  if (!is_number(argv[2])) {
    char err[255];
    sprintf(err, "mem_dump command incorrect number of bytes(%s)\n", argv[2]);
    send(socket, err, strlen(err), 0);
    return;
  }
  long byte_count = get_number(argv[2]);
  if (byte_count > 100) {
    char err[255];
    sprintf(err,
            "the maximum number of received bytes at a time should not exceed "
            "100(%s>100)\n",
            argv[2]);
    send(socket, err, strlen(err), 0);
    return;
  }
  char buf[1024];
  int j = 0;
  for (int i = 0; i < byte_count; i++) {
    sprintf(&buf[j], "0x%02x ", *(uint8_t*)ptr);
    j += 5;
    if (((i + 1) % 8) == 0) {
      ++j;
      sprintf(&buf[j], "\n");
      j += 2;
    }
    ptr++;
  }
  ++j;
  sprintf(&buf[j], "\n");
  send(socket, buf, j, 0);
}

void dbg_memory_write(int socket, int count, char** argv) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}

void dbg_resolve(int socket, int count, char** argv) {
  if (count != 2) {
    char* err = "s command has an incorrect number of arguments\n";
    send(socket, err, strlen(err), 0);
    return;
  }
  char buf[1024];
  if (is_number(argv[1])) {
    long addr = get_number(argv[1]);
    void* ptr = (void*)addr;
    Dl_info info;
    int res = dladdr(ptr, &info);
    if (res != 0) {
      sprintf(buf, "Address '%s' located at %s within the program %s\n",
              argv[1], info.dli_fname, info.dli_sname);
    } else {
      sprintf(buf, "Address '%s' not found\n", argv[1]);
    }
  } else {
    dlerror();
    void* ptr = dlsym(NULL, argv[1]);
    void* error = dlerror();
    if (!error) {
      sprintf(buf, "Symbol '%s' at %p\n", argv[1], ptr);
    } else {
      sprintf(buf, "Symbol '%s' not found\n", argv[1]);
    }
  }

  send(socket, buf, strlen(buf), 0);
}

enum type_ptr_t { UNKNOWN = 0, U8, U16, U32 };

static enum type_ptr_t get_type_ptr(char* argv) {
  if (!strcmp(argv, "u8")) {
    return U8;
  }
  if (!strcmp(argv, "u16")) {
    return U16;
  }
  if (!strcmp(argv, "u32")) {
    return U32;
  }

  return UNKNOWN;
}

uint8_t test_8 = 0x8;
uint16_t test_16 = 0x16;
uint32_t test_32 = 0x32;

void dbg_read(int socket, int count, char** argv) {
  if (count != 3) {
    char* err = "r command has an incorrect number of arguments\n";
    send(socket, err, strlen(err), 0);
    return;
  }
  int type_ptr = get_type_ptr(argv[1]);
  char buf[1024];
  void* ptr = NULL;
  ptr = get_pointer(argv[2]);
  if (!ptr) {
    sprintf(buf, "Address '%s' is NULL\n", argv[2]);
    send(socket, buf, strlen(buf), 0);
    return;
  }
  switch (type_ptr) {
    case UNKNOWN:
      sprintf(buf, "Type '%s' not found\n", argv[1]);
      break;
    case U8:
      sprintf(buf, "Address '%s' = 0x%x\n", argv[2], *(uint8_t*)ptr);
      break;
    case U16:
      sprintf(buf, "Address '%s' = 0x%x\n", argv[2], *(uint16_t*)ptr);
      break;
    case U32:
      sprintf(buf, "Address '%s' = 0x%x\n", argv[2], *(uint32_t*)ptr);
      break;
  }

  send(socket, buf, strlen(buf), 0);
}

void dbg_write(int socket, int count, char** argv) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
  if (count != 4) {
    char* err = "w command has an incorrect number of arguments\n";
    send(socket, err, strlen(err), 0);
    return;
  }
  int type_ptr = get_type_ptr(argv[1]);
  char buf[1024];
  void* ptr = NULL;
  ptr = get_pointer(argv[2]);
  if (!ptr) {
    sprintf(buf, "Address '%s' is NULL\n", argv[2]);
    send(socket, buf, strlen(buf), 0);
    return;
  }
  void* ptr_data = NULL;
  if (is_number(argv[3])) {
    long number = get_number(argv[3]);
    ptr_data = &number;
  } else {
    sprintf(buf, "Data(%s) of the incorrect type\n", argv[3]);
    send(socket, buf, strlen(buf), 0);
    return;
  }

  switch (type_ptr) {
    case UNKNOWN:
      sprintf(buf, "Type '%s' not found\n", argv[1]);
      break;
    case U8:
      *(uint8_t*)ptr = *(uint8_t*)ptr_data;
      sprintf(buf, "Writed address '%s' = 0x%x\n", argv[2], *(uint8_t*)ptr);
      break;
    case U16:
      *(uint16_t*)ptr = *(uint16_t*)ptr_data;
      sprintf(buf, "Writed address '%s' = 0x%x\n", argv[2], *(uint16_t*)ptr);
      break;
    case U32:
      *(uint32_t*)ptr = *(uint32_t*)ptr_data;
      sprintf(buf, "Writed address '%s' = 0x%x\n", argv[2], *(uint32_t*)ptr);
      break;
  }

  send(socket, buf, strlen(buf), 0);
}

void dbg_function(int socket, int count, char** argv) {
  fprintf(stderr, "%s %d\n", __FUNCTION__, __LINE__);
}
