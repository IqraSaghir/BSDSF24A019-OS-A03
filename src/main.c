#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        if ((arglist = tokenize(cmdline)) != NULL) {

            // First check built-in commands
            if (handle_builtin(arglist) == 0) {

                // Execute external commands
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
