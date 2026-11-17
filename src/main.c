#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

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

        // Store command in history
        if (cmdline[0] != '\0') {
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
