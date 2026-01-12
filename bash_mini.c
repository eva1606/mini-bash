#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define DELIMITERS " \t\r\n\a"

// Function to handle external commands (e.g., ls, pwd)
void execute_external(char **args) {
    pid_t pid;
    int status;
    char path[MAX_LINE];
    char *home = getenv("HOME");
    int found = 0;

    // Search for the command in the HOME directory, then in /bin
    if (home != NULL) {
        snprintf(path, sizeof(path), "%s/%s", home, args[0]);
        if (access(path, X_OK) == 0) found = 1;
    }
    if (!found) {
        snprintf(path, sizeof(path), "/bin/%s", args[0]);
        if (access(path, X_OK) == 0) found = 1;
    }

    if (!found) {
        fprintf(stderr, "%s: Unknown Command\n", args[0]);
        return;
    }

    // fork(): Create a child process to execute the command
    pid = fork();
    if (pid == 0) {
        // execv(): Replace the child process with the external program
        if (execv(path, args) == -1) {
            perror("execv error"); // Print error if execv fails
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("fork error"); // Print error if fork fails
    } else {
        // waitpid(): Parent process waits for the child to finish
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            // Display the return code of the terminated process
            printf("Command executed successfully with Return code: %d\n", WEXITSTATUS(status));
        }
    }
}

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        printf("mini-bash$ ");
        fflush(stdout);

        // Read user input
        if (!fgets(line, MAX_LINE, stdin)) break;
        
        // Remove the newline character (\n) from the end of the input
        line[strcspn(line, "\n")] = 0;

        // Parse the input line into separate arguments (Tokens)
        int i = 0;
        args[i] = strtok(line, DELIMITERS);
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            args[++i] = strtok(NULL, DELIMITERS);
        }

        if (args[0] == NULL) continue;

        // Internal command: exit
        if (strcmp(args[0], "exit") == 0) {
            break;
        } 
        // Internal command: cd (change directory)
        else if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "cd: expected argument\n");
            } else {
                // chdir(): System call to change the Shell's working directory
                if (chdir(args[1]) != 0) {
                    perror("cd error"); // Print error if the directory doesn't exist
                }
            }
        } 
        // For everything else, attempt external execution
        else {
            execute_external(args);
        }
    }
    return 0;
}
