
#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>


char *commands[] = {
    "cd", "exit", "help", "jobs", "history",
    "pwd", "ls", "clear", "mkdir", "cat",
    "echo", "grep", "gcc", "nano", "whoami",
    NULL
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

        if (strncmp(name, text, length) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}


char **command_completion(const char *text, int start, int end) {
    (void)end;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    return rl_completion_matches(text, rl_filename_completion_function);
}


/* Execute one command segment */
void run_segment(char** command, int background) {
    if (command == NULL || command[0] == NULL) {
        return;
    }

    if (handle_builtin(command) == 0) {
        execute(command, background);
    }
}


int main(void) {
    char* cmdline;
    char** arglist;

    rl_attempted_completion_function = command_completion;

    while (1) {

        /* Reap completed background processes before each prompt */
        reap_jobs();

        cmdline = readline(PROMPT);

        if (cmdline == NULL) {
            break;
        }

        /* Handle !n history command */
        if (cmdline[0] == '!') {
            char* endptr;
            long number = strtol(cmdline + 1, &endptr, 10);

            if (endptr == cmdline + 1 ||
                *endptr != '\0' || number < 1) {

                printf("Invalid history command.\n");
                free(cmdline);
                continue;
            }

            char* old_command = get_command_history((int)number);

            if (old_command == NULL) {
                printf("No such command in history.\n");
                free(cmdline);
                continue;
            }

            char* new_command = malloc(strlen(old_command) + 1);

            if (new_command == NULL) {
                perror("malloc");
                free(cmdline);
                continue;
            }

            strcpy(new_command, old_command);
            free(cmdline);
            cmdline = new_command;

            printf("%s\n", cmdline);
        }

        if (cmdline[0] != '\0') {
            add_history(cmdline);
            store_command_history(cmdline);
        }

        arglist = tokenize(cmdline);

        if (arglist != NULL) {

            /* Count tokens before inserting NULL separators */
            int token_count = 0;

            while (arglist[token_count] != NULL) {
                token_count++;
            }

            int start = 0;

            for (int i = 0; i <= token_count; i++) {

                if (i == token_count ||
                    strcmp(arglist[i], ";") == 0) {

                    if (i < token_count) {
                        arglist[i] = NULL;
                    }

                    if (start < i) {
                        int background = 0;

                        /* Check whether segment ends with & */
                        if (strcmp(arglist[i - 1], "&") == 0) {
                            arglist[i - 1] = NULL;
                            background = 1;
                        }

                        run_segment(&arglist[start], background);
                    }

                    start = i + 1;
                }
            }

            /* Free every original token */
            for (int i = 0; i < token_count; i++) {
                free(arglist[i]);
            }

            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

