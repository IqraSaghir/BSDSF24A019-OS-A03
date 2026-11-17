
#include "shell.h"

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);

    char* cmdline = malloc(MAX_LEN);

    if (cmdline == NULL) {
        perror("malloc");
        return NULL;
    }

    int c;
    int pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') {
            break;
        }

        if (pos < MAX_LEN - 1) {
            cmdline[pos++] = c;
        }
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL;
    }

    cmdline[pos] = '\0';
    return cmdline;
}


char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0') {
        return NULL;
    }

    char** arglist = calloc(MAXARGS + 1, sizeof(char*));

    if (arglist == NULL) {
        perror("calloc");
        return NULL;
    }

    char* cp = cmdline;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {

        while (*cp == ' ' || *cp == '\t') {
            cp++;
        }

        if (*cp == '\0') {
            break;
        }

        /* Recognize shell operators */
        if (*cp == '<' || *cp == '>' || *cp == '|' ||
            *cp == ';' || *cp == '&') {

            arglist[argnum] = malloc(2);

            if (arglist[argnum] == NULL) {
                perror("malloc");
                break;
            }

            arglist[argnum][0] = *cp;
            arglist[argnum][1] = '\0';

            cp++;
            argnum++;
        }
        else {
            char* start = cp;

            while (*cp != '\0' &&
                   *cp != ' ' && *cp != '\t' &&
                   *cp != '<' && *cp != '>' &&
                   *cp != '|' && *cp != ';' &&
                   *cp != '&') {
                cp++;
            }

            int len = cp - start;

            arglist[argnum] = malloc(len + 1);

            if (arglist[argnum] == NULL) {
                perror("malloc");
                break;
            }

            strncpy(arglist[argnum], start, len);
            arglist[argnum][len] = '\0';

            argnum++;
        }
    }

    if (argnum == 0) {
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

