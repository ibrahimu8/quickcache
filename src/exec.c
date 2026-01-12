#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 512

// Simple scan to see if any source file uses math functions
static int needs_math_lib(char **argv) {
    for (int i = 1; argv[i]; i++) {
        if (strstr(argv[i], ".c")) {  // only scan C source files
            FILE *f = fopen(argv[i], "r");
            if (!f) continue;

            char line[4096];
            while (fgets(line, sizeof(line), f)) {
                if (strstr(line, "pow(") || strstr(line, "sqrt(") ||
                    strstr(line, "sin(") || strstr(line, "cos(") ||
                    strstr(line, "tan(") || strstr(line, "log(")) {
                    fclose(f);
                    return 1;
                }
            }
            fclose(f);
        }
    }
    return 0;
}

int execute_compiler(char **argv) {
    // Check if we need -lm
    int add_math = needs_math_lib(argv);

    char *final_args[MAX_ARGS];
    int j = 0;
    for (; argv[j] && j < MAX_ARGS - 2; j++) {  // leave space for -lm + NULL
        final_args[j] = argv[j];
    }

    if (add_math) {
        final_args[j++] = "-lm";
    }
    final_args[j] = NULL;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        execvp(final_args[0], final_args);
        perror("execvp");
        exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}
