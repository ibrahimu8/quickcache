#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int execute_compiler(char **argv) {
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        execvp(argv[0], argv);
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
