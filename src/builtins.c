
#include "shell.h"

typedef struct Job {
    pid_t pid;
    struct Job* next;
} Job;

static Job* job_list = NULL;


/* Add a background process to the job list */
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
                if (previous == NULL) {
                    job_list = current->next;
                }
                else {
                    previous->next = current->next;
                }

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


int handle_builtin(char** arglist) {

    if (arglist == NULL || arglist[0] == NULL) {
        return 1;
    }

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            printf("cd: missing directory\n");
        }
        else if (chdir(arglist[1]) != 0) {
            perror("cd");
        }

        return 1;
    }

    if (strcmp(arglist[0], "help") == 0) {
        printf("Available built-in commands:\n");
        printf("exit    - Exit the shell\n");
        printf("cd      - Change directory\n");
        printf("help    - Display help\n");
        printf("jobs    - Show background jobs\n");
        printf("history - Display command history\n");

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

    return 0;
}

