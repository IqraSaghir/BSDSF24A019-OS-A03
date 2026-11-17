#include "shell.h"

#define HISTORY_SIZE 20

char *command_history[HISTORY_SIZE];
int history_count = 0;

// Store a command in history
void store_command_history(const char* command) {

    // If history is full, remove the oldest command
    if (history_count == HISTORY_SIZE) {

        free(command_history[0]);

        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            command_history[i] = command_history[i + 1];
        }

        history_count--;
    }

    // Allocate memory and copy command
    command_history[history_count] = malloc(strlen(command) + 1);

    if (command_history[history_count] != NULL) {
        strcpy(command_history[history_count], command);
        history_count++;
    }
}

// Display saved commands
void display_command_history() {

    for (int i = 0; i < history_count; i++) {
        printf("%d  %s\n", i + 1, command_history[i]);
    }
}

// Get command by its history number
char* get_command_history(int number) {

    if (number < 1 || number > history_count) {
        return NULL;
    }

    return command_history[number - 1];
}
