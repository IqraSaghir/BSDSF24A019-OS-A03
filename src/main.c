
#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>



// Commands available for completion
char *commands[] = {
    "cd",
    "exit",
    "help",
    "jobs",
    "history",
    "pwd",
    "ls",
    "clear",
    "mkdir",
    "cat",
    "echo",
    "grep",
    "gcc",
    "nano",
    "whoami",
    NULL
};

// Generate command suggestions
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

// Handle command and filename completion
char **command_completion(const char *text, int start, int end) {

    (void)end;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    return rl_completion_matches(text, rl_filename_completion_function);
}


int main() {
    char* cmdline;
    char** arglist;


rl_attempted_completion_function = command_completion;

    while ((cmdline = readline(PROMPT)) != NULL) {

        // Check if user entered !n
        if (cmdline[0] == '!') {

            char* endptr;
            long number = strtol(cmdline + 1, &endptr, 10);

            if (endptr == cmdline + 1 || *endptr != '\0' || number < 1) {
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

            // Copy the previous command
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

        // Add non-empty command to Readline history
        if (cmdline[0] != '\0') {
            add_history(cmdline);
            store_command_history(cmdline);
        }

        // Tokenize command
        if ((arglist = tokenize(cmdline)) != NULL) {

            // Check built-in commands first
            if (handle_builtin(arglist) == 0) {
                execute(arglist);
            }

            // Free allocated memory
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }

            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

