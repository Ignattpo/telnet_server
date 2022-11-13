# telnet_server
Создание и освобождение telnet сервера:
```
int main() {
  struct telnet_dbg_t* dbg = telnet_dbg_init("localhost", 2323); //иницилизируется сервер на адресс "localhost" и порт socket

  telnet_dbg_run(dbg);//запускается поток работы сервера
  
  ........................

  telnet_dbg_stop(dbg);//останавливается поток работы сервера,происходит ожидание завершения как сервера так и всех клиентов 

  telnet_dbg_free(dbg);//освобождаются данные принадлежащие серверу 

  return 0;
}
````
Команды обработываемые сервером
```
^]-отключение от сервера.


mem_dump 0x2378423 64 - получить содержимое памяти по указателю 
mem_dump g_some_var 4 

mem_dump - команда
0x2378423/g_some_var - адресс начала памяти 
64 - количество байт необходимых считать(максимально 100)

Пример вывода:
0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00


mem_write 0x2378423 0x01 0x02 0xFF 0xAA 0xCC - записать данные в память по указателю 
mem_write g_some_var 23 10

mem_write  - команда
0x2378423/g_some_var - адресс начала памяти 
0x01 0x02 0xFF 0xAA 0xCC/23 10 - данные для записи, возможно использовать только числа

Пример вывода:
writed 5 bytes to address 0x2378423


s malloc - резолвим имя переменной/функции
s g_some_var

s - команда
malloc/g_some_var -имя функции/переменной

Пример вывода:
symbol ‘malloc’ at 0x76cadc9

s 0x76cadc9 - резолвим функцию по адресу

s - команда
0x76cadc9 - адресс функции

Пример вывода:
Address '0xf7de5fb0' located at /lib/i386-linux-gnu/libc.so.6 within the program __libc_malloc


r u32 g_some_var - читаем переменную (u8 u16 u32)
r u32 0x76cfff

s - команда
u32 - тип переменной, поддерживаются u8 u16 u32
g_some_var/0x76cfff - имя/адресс переменной

Пример вывода:
Address '0x76cfff' = 0x20
Address 'g_some_var' = 0x20


w u32 g_some_var 0x20 - пишем в переменную по адресу/имени
w u32 0x76cfff 0xda

w - команда
u32 - тип переменной, поддерживаются u8 u16 u32
g_some_var/0x76cfff - имя/адресс переменной
0x20/0xda - данные для записи, возможно использовать только числа

Пример вывода:
Writed address 'g_some_var' = 0x20
Writed address '0x76cfff' = 0x20


c malloc 0xff - запускаем функцию
c strlen Helllo
c 0x76caabb "Hello word"

c - команда
malloc/strlen/0x76caabb - имя/адресс вызываемой функции 
0xff/Helllo/"Hello word" - аргументы функции, поддерживается до 5 аргументов включительно, строки имеющие пробелы необходимо заключать а "" 

Пример вывода:
c malloc 0xff
Function ‘malloc’ at 0x76cadc9 returned 0xFA273A2
Function ‘strlen’ at 0x76caabb returned 0x6

```

