
#include "shell.h"

typedef struct Job {
    pid_t pid;
    struct Job* next;
} Job;

typedef struct Variable {
    char* name;
    char* value;
    struct Variable* next;
} Variable;

static Job* job_list = NULL;
static Variable* variable_list = NULL;

/* Add background process */
void add_job(pid_t pid) {
    Job* new_job = malloc(sizeof(Job));

    if (new_job == NULL) {
        perror("malloc");
        return;
    }

    new_job->pid = pid;
    new_job->next = job_list;
    job_list = new_job;

    printf("[Background process started] PID: %d\n", pid);
}

/* Remove completed background processes */
void reap_jobs(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        Job* current = job_list;
        Job* previous = NULL;

        while (current != NULL) {
            if (current->pid == pid) {
                if (previous == NULL)
                    job_list = current->next;
                else
                    previous->next = current->next;

                printf("\n[Completed] PID: %d\n", pid);
                free(current);
                break;
            }

            previous = current;
            current = current->next;
        }
    }
}

/* Display background jobs */
void display_jobs(void) {
    reap_jobs();

    Job* current = job_list;

    if (current == NULL) {
        printf("No running background jobs.\n");
        return;
    }

    printf("Background jobs:\n");

    while (current != NULL) {
        printf("PID: %d  Status: Running\n", current->pid);
        current = current->next;
    }
}

/* Check whether command is a variable assignment */
int is_variable_assignment(const char* command) {
    if (command == NULL)
        return 0;

    const char* equal = strchr(command, '=');

    if (equal == NULL || equal == command)
        return 0;

    if (!(isalpha((unsigned char)command[0]) ||
          command[0] == '_'))
        return 0;

    for (const char* p = command; p < equal; p++) {
        if (!(isalnum((unsigned char)*p) || *p == '_'))
            return 0;
    }

    return 1;
}

/* Create or update a shell variable */
int set_shell_variable(const char* assignment) {
    if (!is_variable_assignment(assignment)) {
        printf("Invalid variable assignment.\n");
        return 0;
    }

    const char* equal = strchr(assignment, '=');
    size_t name_length = equal - assignment;

    char* name = malloc(name_length + 1);

    if (name == NULL) {
        perror("malloc");
        return 0;
    }

    strncpy(name, assignment, name_length);
    name[name_length] = '\0';

    const char* value_start = equal + 1;
    size_t value_length = strlen(value_start);

    char* value = malloc(value_length + 1);

    if (value == NULL) {
        perror("malloc");
        free(name);
        return 0;
    }

    strcpy(value, value_start);

    /* Remove matching surrounding quotes */
    if (value_length >= 2 &&
        ((value[0] == '"' && value[value_length - 1] == '"') ||
         (value[0] == '\'' && value[value_length - 1] == '\''))) {
        memmove(value, value + 1, value_length - 2);
        value[value_length - 2] = '\0';
    }

    Variable* current = variable_list;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            free(current->value);
            current->value = value;
            free(name);
            return 1;
        }

        current = current->next;
    }

    Variable* new_variable = malloc(sizeof(Variable));

    if (new_variable == NULL) {
        perror("malloc");
        free(name);
        free(value);
        return 0;
    }

    new_variable->name = name;
    new_variable->value = value;
    new_variable->next = variable_list;
    variable_list = new_variable;

    return 1;
}

/* Display all shell variables */
void display_shell_variables(void) {
    Variable* current = variable_list;

    if (current == NULL) {
        printf("No shell variables defined.\n");
        return;
    }

    while (current != NULL) {
        printf("%s=%s\n", current->name, current->value);
        current = current->next;
    }
}

/* Expand $VARIABLE inside command arguments */
void expand_variables(char** arglist) {
    if (arglist == NULL)
        return;

    for (int i = 0; arglist[i] != NULL; i++) {
        char* original = arglist[i];
        size_t capacity = strlen(original) + 1;
        char* expanded = malloc(capacity);

        if (expanded == NULL) {
            perror("malloc");
            continue;
        }

        size_t used = 0;

        for (size_t j = 0; original[j] != '\0';) {
            if (original[j] == '$' &&
                (isalpha((unsigned char)original[j + 1]) ||
                 original[j + 1] == '_')) {

                size_t start = j + 1;
                size_t end = start;

                while (isalnum((unsigned char)original[end]) ||
                       original[end] == '_')
                    end++;

                size_t name_length = end - start;
                char* name = malloc(name_length + 1);

                if (name == NULL) {
                    perror("malloc");
                    break;
                }

                strncpy(name, original + start, name_length);
                name[name_length] = '\0';

                Variable* current = variable_list;
                const char* value = "";

                while (current != NULL) {
                    if (strcmp(current->name, name) == 0) {
                        value = current->value;
                        break;
                    }

                    current = current->next;
                }

                size_t value_length = strlen(value);

                if (used + value_length + 1 > capacity) {
                    capacity = used + value_length + 1;
                    char* temp = realloc(expanded, capacity);

                    if (temp == NULL) {
                        perror("realloc");
                        free(name);
                        free(expanded);
                        expanded = NULL;
                        break;
                    }

                    expanded = temp;
                }

                memcpy(expanded + used, value, value_length);
                used += value_length;
                j = end;
                free(name);
            }
            else {
                if (used + 2 > capacity) {
                    capacity = used + 2;
                    char* temp = realloc(expanded, capacity);

                    if (temp == NULL) {
                        perror("realloc");
                        free(expanded);
                        expanded = NULL;
                        break;
                    }

                    expanded = temp;
                }

                expanded[used++] = original[j++];
            }
        }

        if (expanded != NULL) {
            expanded[used] = '\0';
            free(original);
            arglist[i] = expanded;
        }
    }
}

/* Built-in command handler */
int handle_builtin(char** arglist) {
    if (arglist == NULL || arglist[0] == NULL)
        return 1;

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL)
            printf("cd: missing directory\n");
        else if (chdir(arglist[1]) != 0)
            perror("cd");

        return 1;
    }

    if (strcmp(arglist[0], "help") == 0) {
        printf("Available built-in commands:\n");
        printf("exit    - Exit the shell\n");
        printf("cd      - Change directory\n");
        printf("help    - Display help\n");
        printf("jobs    - Show background jobs\n");
        printf("history - Display command history\n");
        printf("set     - Display shell variables\n");

        return 1;
    }

    if (strcmp(arglist[0], "jobs") == 0) {
        display_jobs();
        return 1;
    }

    if (strcmp(arglist[0], "history") == 0) {
        display_command_history();
        return 1;
    }

    if (strcmp(arglist[0], "set") == 0) {
        display_shell_variables();
        return 1;
    }

    if (is_variable_assignment(arglist[0])) {
        set_shell_variable(arglist[0]);
        return 1;
    }

    return 0;
}

