#ifndef REPL_H
#define REPL_H

#define MAX_ARGS 3
#define MAX_ARG_LEN 10

#include <stdbool.h>

typedef enum {
  REPL_EXIT,
  REPL_CONTINUE
} REPL_Status;

REPL_Status repl(REPL_Status (* commandHandler) (int, char[MAX_ARGS][MAX_ARG_LEN]));
bool eq(char* str1, char* str2);
void output(char* msg);

#endif 
