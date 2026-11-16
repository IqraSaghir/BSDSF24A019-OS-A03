#include "shell.h"

int handle_builtin(char** arglist) {

    // Check exit command
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    // Check cd command
    if (strcmp(arglist[0], "cd") == 0) {

        if (arglist[1] == NULL) {
            printf("cd: missing directory\n");
        }
        else {
            if (chdir(arglist[1]) != 0) {
                perror("cd");
            }
        }

        return 1;
    }

    // Check help command
    if (strcmp(arglist[0], "help") == 0) {
        printf("Available built-in commands:\n");
        printf("exit - Exit the shell\n");
        printf("cd - Change current directory\n");
        printf("help - Display help information\n");
        printf("jobs - Show running jobs (placeholder)\n");

        return 1;
    }

    // Check jobs command
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control is not implemented yet.\n");
        return 1;
    }

    return 0;
}
