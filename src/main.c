
#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

char *commands[] = {
    "cd", "exit", "help", "jobs", "history", "set",
    "pwd", "ls", "clear", "mkdir", "cat",
    "echo", "grep", "gcc", "nano", "whoami",
    "true", "false", NULL
};

char *command_generator(const char *text, int state) {
    static int index;
    static int length;

    if (state == 0) {
        index = 0;
        length = strlen(text);
    }

    while (commands[index] != NULL) {
        char *name = commands[index++];

        if (strncmp(name, text, length) == 0)
            return strdup(name);
    }

    return NULL;
}

char **command_completion(const char *text, int start, int end) {
    (void)end;

    if (start == 0)
        return rl_completion_matches(text, command_generator);

    return rl_completion_matches(text, rl_filename_completion_function);
}


void run_segment(char **command, int background) {
    if (command == NULL || command[0] == NULL)
        return;

    expand_variables(command);

    if (handle_builtin(command) == 0)
        execute(command, background);
}







/* Run a simple command and return its exit status */

int run_condition(char *condition) {
    char **args = tokenize(condition);

    if (args == NULL)
        return 1;

    expand_variables(args);
    int result = execute(args, 0);

    for (int i = 0; args[i] != NULL; i++)
        free(args[i]);

    free(args);

    return result;
}














/* Execute an if-then-else-fi block */
void handle_if_block(char *condition) {
    char *then_block = NULL;
    char *else_block = NULL;
    char *line;
    size_t then_size = 1;
    size_t else_size = 1;
    int in_else = 0;
    int found_then = 0;
    int found_fi = 0;

    then_block = calloc(1, 1);
    else_block = calloc(1, 1);

    if (then_block == NULL || else_block == NULL) {
        perror("calloc");
        free(then_block);
        free(else_block);
        return;
    }

    while ((line = readline("IF> ")) != NULL) {
        if (strcmp(line, "then") == 0) {
            found_then = 1;
            free(line);
            continue;
        }

        if (strcmp(line, "else") == 0) {
            in_else = 1;
            free(line);
            continue;
        }

        if (strcmp(line, "fi") == 0) {
            found_fi = 1;
            free(line);
            break;
        }

        if (!found_then) {
            printf("Expected then\n");
            free(line);
            continue;
        }

        char **target = in_else ? &else_block : &then_block;
        size_t *size = in_else ? &else_size : &then_size;

        size_t old_len = strlen(*target);
        size_t new_len = strlen(line);

        char *temp = realloc(*target, old_len + new_len + 2);

        if (temp == NULL) {
            perror("realloc");
            free(line);
            break;
        }

        *target = temp;
        memcpy(*target + old_len, line, new_len);
        (*target)[old_len + new_len] = ';';
        (*target)[old_len + new_len + 1] = '\0';

        *size = old_len + new_len + 2;
        free(line);
    }

    if (!found_then || !found_fi) {
        printf("Invalid if block: missing then or fi\n");
        free(then_block);
        free(else_block);
        return;
    }

    int status = run_condition(condition);

    char *selected = (status == 0) ? then_block : else_block;
    char **commands_list = tokenize(selected);

    if (commands_list != NULL) {
        int count = 0;

        while (commands_list[count] != NULL)
            count++;

        int start = 0;

        for (int i = 0; i <= count; i++) {
            if (i == count || strcmp(commands_list[i], ";") == 0) {
                if (i < count)
                    commands_list[i] = NULL;

                if (start < i)
                    run_segment(&commands_list[start], 0);

                start = i + 1;
            }
        }

        for (int i = 0; i < count; i++)
            free(commands_list[i]);

        free(commands_list);
    }

    free(then_block);
    free(else_block);
}

int main(void) {
    char *cmdline;
    char **arglist;

    rl_attempted_completion_function = command_completion;

    while (1) {
        reap_jobs();

        cmdline = readline(PROMPT);

        if (cmdline == NULL)
            break;

        if (cmdline[0] == '!') {
            char *endptr;
            long number = strtol(cmdline + 1, &endptr, 10);

            if (endptr == cmdline + 1 || *endptr != '\0' || number < 1) {
                printf("Invalid history command.\n");
                free(cmdline);
                continue;
            }

            char *old_command = get_command_history((int)number);

            if (old_command == NULL) {
                printf("No such command in history.\n");
                free(cmdline);
                continue;
            }

            char *new_command = strdup(old_command);
            free(cmdline);
            cmdline = new_command;

            if (cmdline == NULL) {
                perror("strdup");
                continue;
            }

            printf("%s\n", cmdline);
        }

        if (cmdline[0] != '\0') {
            add_history(cmdline);
            store_command_history(cmdline);
        }

        if (strncmp(cmdline, "if ", 3) == 0) {
            handle_if_block(cmdline + 3);
            free(cmdline);
            continue;
        }

        arglist = tokenize(cmdline);

        if (arglist != NULL) {
            int count = 0;

            while (arglist[count] != NULL)
                count++;

            int start = 0;

            for (int i = 0; i <= count; i++) {
                if (i == count || strcmp(arglist[i], ";") == 0) {
                    if (i < count)
                        arglist[i] = NULL;

                    if (start < i) {
                        int background = 0;

                        if (strcmp(arglist[i - 1], "&") == 0) {
                            arglist[i - 1] = NULL;
                            background = 1;
                        }

                        run_segment(&arglist[start], background);
                    }

                    start = i + 1;
                }
            }

            for (int i = 0; i < count; i++)
                free(arglist[i]);

            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

