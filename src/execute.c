
#include "shell.h"
#include <fcntl.h>

int execute(char* arglist[]) {
    int status;
    int input_fd = -1;
    int output_fd = -1;
    int pipe_fd[2];

    int pipe_pos = -1;
    int input_pos = -1;
    int output_pos = -1;

    /* Find special symbols */
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
        arglist[pipe_pos] = NULL;

        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            return -1;
        }

        pid_t pid1 = fork();

        if (pid1 == 0) {
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);

            execvp(arglist[0], arglist);
            perror("Command failed");
            exit(1);
        }

        pid_t pid2 = fork();

        if (pid2 == 0) {
            close(pipe_fd[1]);
            dup2(pipe_fd[0], STDIN_FILENO);
            close(pipe_fd[0]);

            execvp(arglist[pipe_pos + 1],
                   &arglist[pipe_pos + 1]);

            perror("Command failed");
            exit(1);
        }

        close(pipe_fd[0]);
        close(pipe_fd[1]);

        waitpid(pid1, &status, 0);
        waitpid(pid2, &status, 0);

        return 0;
    }

    /* Handle input redirection */
    if (input_pos != -1) {
        input_fd = open(arglist[input_pos + 1], O_RDONLY);

        if (input_fd == -1) {
            perror("Input file");
            return -1;
        }

        arglist[input_pos] = NULL;
    }

    /* Handle output redirection */
    if (output_pos != -1) {
        output_fd = open(arglist[output_pos + 1],
                         O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (output_fd == -1) {
            perror("Output file");
            if (input_fd != -1)
                close(input_fd);
            return -1;
        }

        arglist[output_pos] = NULL;
    }

    pid_t cpid = fork();

    if (cpid == -1) {
        perror("fork failed");
        if (input_fd != -1) close(input_fd);
        if (output_fd != -1) close(output_fd);
        return -1;
    }

    if (cpid == 0) {

        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execvp(arglist[0], arglist);
        perror("Command failed");
        exit(1);
    }

    if (input_fd != -1) close(input_fd);
    if (output_fd != -1) close(output_fd);

    waitpid(cpid, &status, 0);

    return 0;
}

