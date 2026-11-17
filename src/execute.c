
#include "shell.h"
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

/* Add background process to job list */
extern void add_job(pid_t pid);

/* Execute command with optional pipe and redirection */
int execute(char **arglist, int background) {
    if (arglist == NULL || arglist[0] == NULL)
        return 0;

    int pipe_pos = -1;
    int input_pos = -1;
    int output_pos = -1;

    /* Find pipe and redirection operators */
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0)
            pipe_pos = i;
        else if (strcmp(arglist[i], "<") == 0)
            input_pos = i;
        else if (strcmp(arglist[i], ">") == 0)
            output_pos = i;
    }

    /* Handle pipe */
    if (pipe_pos != -1) {
        int fd[2];

        if (pipe(fd) == -1) {
            perror("pipe");
            return 1;
        }

        arglist[pipe_pos] = NULL;
        char **left_cmd = arglist;
        char **right_cmd = &arglist[pipe_pos + 1];

        pid_t pid1 = fork();

        if (pid1 < 0) {
            perror("fork");
            close(fd[0]);
            close(fd[1]);
            return 1;
        }

        if (pid1 == 0) {
            close(fd[0]);

            if (dup2(fd[1], STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }

            close(fd[1]);
            execvp(left_cmd[0], left_cmd);
            perror("execvp");
            exit(127);
        }

        pid_t pid2 = fork();

        if (pid2 < 0) {
            perror("fork");
            close(fd[0]);
            close(fd[1]);
            waitpid(pid1, NULL, 0);
            return 1;
        }

        if (pid2 == 0) {
            close(fd[1]);

            if (dup2(fd[0], STDIN_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }

            close(fd[0]);
            execvp(right_cmd[0], right_cmd);
            perror("execvp");
            exit(127);
        }

        close(fd[0]);
        close(fd[1]);

        int status1;
        int status2;

        waitpid(pid1, &status1, 0);

        if (background) {
            add_job(pid2);
            return 0;
        }

        waitpid(pid2, &status2, 0);

        if (WIFEXITED(status2))
            return WEXITSTATUS(status2);

        return 1;
    }

    /* Handle input and output redirection */
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Input redirection */
        if (input_pos != -1) {
            if (arglist[input_pos + 1] == NULL) {
                fprintf(stderr, "Missing input filename\n");
                exit(1);
            }

            int fd = open(arglist[input_pos + 1], O_RDONLY);

            if (fd < 0) {
                perror("open");
                exit(1);
            }

            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2");
                close(fd);
                exit(1);
            }

            close(fd);
            arglist[input_pos] = NULL;
        }

        /* Output redirection */
        if (output_pos != -1) {
            if (arglist[output_pos + 1] == NULL) {
                fprintf(stderr, "Missing output filename\n");
                exit(1);
            }

            int fd = open(arglist[output_pos + 1],
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (fd < 0) {
                perror("open");
                exit(1);
            }

            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2");
                close(fd);
                exit(1);
            }

            close(fd);
            arglist[output_pos] = NULL;
        }

        execvp(arglist[0], arglist);
        perror("execvp");
        exit(127);
    }

    /* Background execution */
    if (background) {
        add_job(pid);
        return 0;
    }

    /* Foreground execution */
    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);

    return 1;
}

