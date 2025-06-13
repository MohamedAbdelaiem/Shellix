#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_TOKENS 100
#define MAX_PATHS 10

char **search_paths;
int path_count;
pid_t pipeline_pgid;

void handle_signal(int sig) {
    if (pipeline_pgid != 0) {
        killpg(pipeline_pgid, sig);
    }
}

char **tokenize_input(char *line, int *token_count) {
    char **tokens = malloc(MAX_TOKENS * sizeof(char *));
    char *current_token = malloc(MAX_LINE);
    int token_index = 0;
    int in_quotes = 0;
    char quote_type = '\0';

    *token_count = 0;

    for (char *ptr = line; *ptr; ptr++) {
        if (in_quotes) {
            if (*ptr == quote_type) {
                in_quotes = 0;
                quote_type = '\0';
            } else {
                current_token[token_index++] = *ptr;
            }
        } else {
            if (*ptr == '"' || *ptr == '\'') {
                in_quotes = 1;
                quote_type = *ptr;
            } else if (*ptr == ' ' || *ptr == '\t') {
                if (token_index > 0) {
                    current_token[token_index] = '\0';
                    tokens[(*token_count)++] = strdup(current_token);
                    token_index = 0;
                }
            } else if (*ptr == '|' || *ptr == '>') {
                if (token_index > 0) {
                    current_token[token_index] = '\0';
                    tokens[(*token_count)++] = strdup(current_token);
                    token_index = 0;
                }
                tokens[(*token_count)++] = strndup(ptr, 1);
            } else {
                current_token[token_index++] = *ptr;
            }
        }
    }
    if (token_index > 0) {
        current_token[token_index] = '\0';
        tokens[(*token_count)++] = strdup(current_token);
    }
    free(current_token);
    return tokens;
}

char *find_executable(char *command) {
    if (path_count == 0) return NULL;
    for (int i = 0; i < path_count; i++) {
        char *full_path = malloc(strlen(search_paths[i]) + strlen(command) + 2);
        sprintf(full_path, "%s/%s", search_paths[i], command);
        if (access(full_path, X_OK) == 0) return full_path;
        free(full_path);
    }
    return NULL;
}

void run_pwd() {
    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        fflush(stdout);
    } else {
        fprintf(stderr, "An error has occurred!\n");
    }
}

int run_builtin(char **args, int arg_count) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (arg_count != 2) {
            fprintf(stderr, "An error has occurred!\n");
            return 1;
        }
        if (chdir(args[1]) != 0) {
            fprintf(stderr, "An error has occurred!\n");
        }
        return 1;
    } else if (strcmp(args[0], "pwd") == 0) {
        run_pwd();
        return 1;
    } else if (strcmp(args[0], "path") == 0) {
        for (int i = 0; i < path_count; i++) free(search_paths[i]);
        free(search_paths);
        path_count = arg_count - 1;
        search_paths = malloc(path_count * sizeof(char *));
        for (int i = 0; i < path_count; i++) {
            search_paths[i] = strdup(args[i + 1]);
        }
        return 1;
    }
    return 0;
}

void run_command(char **args, int input_fd, int output_fd) {
    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    if (output_fd != STDOUT_FILENO) {
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }
    if (strcmp(args[0], "pwd") == 0) {
        run_pwd();
        exit(0);
    }
    char *full_path = find_executable(args[0]);
    if (!full_path) {
        fprintf(stderr, "An error has occurred!\n");
        exit(1);
    }
    execv(full_path, args);
    fprintf(stderr, "An error has occurred!\n");
    exit(1);
}

void run_pipeline(char ***commands, int command_count, char *output_file) {
    int pipes[command_count - 1][2];
    pid_t pids[command_count];

    for (int i = 0; i < command_count - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            fprintf(stderr, "An error has occurred!\n");
            exit(1);
        }
    }

    for (int i = 0; i < command_count; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i > 0) dup2(pipes[i - 1][0], STDIN_FILENO);
            if (i < command_count - 1) dup2(pipes[i][1], STDOUT_FILENO);
            else if (output_file) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    fprintf(stderr, "An error has occurred!\n");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            for (int j = 0; j < command_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            run_command(commands[i], STDIN_FILENO, STDOUT_FILENO);
        } else if (pids[i] > 0) {
            if (i == 0) {
                pipeline_pgid = pids[0];
                setpgid(pids[0], pids[0]);
            } else {
                setpgid(pids[i], pipeline_pgid);
            }
        } else {
            fprintf(stderr, "An error has occurred!\n");
            exit(1);
        }
    }

    for (int i = 0; i < command_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < command_count; i++) {
        waitpid(pids[i], NULL, 0);
    }
    pipeline_pgid = 0;
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    int is_interactive = 1;

    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "An error has occurred!\n");
            exit(1);
        }
        is_interactive = 0;
    } else if (argc > 2) {
        fprintf(stderr, "An error has occurred!\n");
        exit(1);
    }

    path_count = 1;
    search_paths = malloc(path_count * sizeof(char *));
    search_paths[0] = strdup("/bin");

    signal(SIGINT, handle_signal);
    signal(SIGTSTP, handle_signal);

    char line[MAX_LINE];
    while (1) {
        if (is_interactive) {
            printf("cmpsh> ");
            fflush(stdout);
        }
        if (!fgets(line, sizeof(line), input)) exit(0);
        line[strcspn(line, "\n")] = 0;

        int token_count;
        char **tokens = tokenize_input(line, &token_count);
        if (token_count == 0) {
            free(tokens);
            continue;
        }

        char *output_file = NULL;
        for (int i = 0; i < token_count - 1; i++) {
            if (strcmp(tokens[i], ">") == 0) {
                if (!output_file && i + 1 < token_count) {
                    output_file = tokens[i + 1];
                    token_count -= 2;
                    for (int j = i; j < token_count; j++) tokens[j] = tokens[j + 2];
                    break;
                } else {
                    fprintf(stderr, "An error has occurred!\n");
                    break;
                }
            }
        }

        char ***commands = malloc(MAX_TOKENS * sizeof(char **));
        int command_count = 0;
        int cmd_start = 0;
        for (int i = 0; i <= token_count; i++) {
            if (i == token_count || strcmp(tokens[i], "|") == 0) {
                if (i > cmd_start) {
                    commands[command_count] = malloc((i - cmd_start + 1) * sizeof(char *));
                    for (int j = cmd_start; j < i; j++) {
                        commands[command_count][j - cmd_start] = tokens[j];
                    }
                    commands[command_count][i - cmd_start] = NULL;
                    command_count++;
                }
                cmd_start = i + 1;
            }
        }

        if (command_count == 0) {
            for (int i = 0; i < token_count; i++) free(tokens[i]);
            free(tokens);
            free(commands);
            continue;
        }

        int has_pipe = command_count > 1;
        for (int i = 0; i < command_count; i++) {
            char *cmd = commands[i][0];
            if (has_pipe && (strcmp(cmd, "cd") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "paths") == 0)) {
                fprintf(stderr, "An error has occurred!\n");
                goto cleanup;
            }
        }

        if (command_count == 1 && !output_file && !has_pipe) {
            char **args = commands[0];
            if (!run_builtin(args, token_count)) {
                pid_t pid = fork();
                if (pid == 0) {
                    run_command(args, STDIN_FILENO, STDOUT_FILENO);
                } else if (pid > 0) {
                    pipeline_pgid = pid;
                    setpgid(pid, pid);
                    waitpid(pid, NULL, 0);
                    pipeline_pgid = 0;
                } else {
                    fprintf(stderr, "An error has occurred!\n");
                }
            }
        } else {
            run_pipeline(commands, command_count, output_file);
        }

    cleanup:
        for (int i = 0; i < command_count; i++) free(commands[i]);
        free(commands);
        for (int i = 0; i < token_count; i++) free(tokens[i]);
        free(tokens);
    }

    for (int i = 0; i < path_count; i++) free(search_paths[i]);
    free(search_paths);
    return 0;
}