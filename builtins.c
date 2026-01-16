#include <stdio.h>   // For printf, fprintf
#include <stdlib.h>  // For exit()
#include <string.h>  // For strcmp()
#include <unistd.h>  // For chdir() (when you implement cd)
#include <builtins.h>

int builtin_exit(char **args) {
    exit(0);
    return 1;
}

int builtin_cd(char **args) {
    char *path;

    if (args[1] == NULL || strcmp(args[1], "~") == 0) {
        path = getenv("HOME");
        if (path == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    } else {
        path = args[1];
    }

    if (chdir(path) != 0) {
        perror("cd");
    }
    return 1;
}

int builtin_help(char ** args) {
    return(1);
}

char *builtin_commands[] = {
    "exit",
    "cd",
    "help"
};

int num_builtins = sizeof(builtin_commands) / sizeof(char *);

int (*builtin_functions[])(char **) = {
    &builtin_exit,
    &builtin_cd,
    &builtin_help
};

int execute_builtin(char **args){
    for (int i = 0; i < num_builtins; i++) {
        if (strcmp(args[0], builtin_commands[i]) == 0) {
            return builtin_functions[i](args);
        }
    }
    return 0;
}