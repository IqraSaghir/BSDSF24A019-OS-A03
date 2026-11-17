
#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "

char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist, int background);
int handle_builtin(char** arglist);

void store_command_history(const char* command);
void display_command_history(void);
char* get_command_history(int number);

void add_job(pid_t pid);
void reap_jobs(void);
void display_jobs(void);

/* Shell variable functions */
void expand_variables(char** arglist);
int is_variable_assignment(const char* command);
int set_shell_variable(const char* assignment);
void display_shell_variables(void);

#endif

