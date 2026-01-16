#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "builtins.h"

int
main(int argc, char *argv[])
{
	char *line = NULL;
	size_t len = 0;

	int mem_size = 16;
	char **parsed_args = malloc(mem_size * sizeof(char *));

	// -------------------------Parsing arguments------------------------- //
	while(1){
		printf("dacoolestshell> ");
		fflush(stdout);

		getline(&line, &len, stdin);
		
		char *arg;
		arg = strtok(line, " \n");
		int ctr = 1;

		while(arg){
			parsed_args[ctr-1] = arg;
			ctr++;
			arg = strtok(NULL, " \n");

			if(ctr >= mem_size){
				char **doubled_mem = realloc(parsed_args, mem_size * 2 * sizeof(char *));
				if (doubled_mem == NULL) {
					fprintf(stderr, "Memory reallocation failed.\n");
					free(parsed_args);
					exit(1);
				}
				parsed_args = doubled_mem;
				mem_size = mem_size * 2;
			}
		}
		parsed_args[ctr-1] = NULL;

		// -------------------------Executing command------------------------- //

		// Splitting commands if piped input
		int num_commands = 1;
		char **commands[10];
		commands[0] = parsed_args;

		// Redirection vars
		int number_args = ctr - 1;
		char *output_file = NULL;
		char *input_file = NULL;
		int is_append = 0;
		int has_error = 0;

		for (int i = 0; i < number_args; i++) {
			if (strcmp(parsed_args[i], "|") == 0) {
				parsed_args[i] = NULL;
				commands[num_commands] = &parsed_args[i + 1];
				num_commands++;
			}
		}

		if (num_commands == 1 && execute_builtin(parsed_args)) {
			continue;
		}

		// Creating pipes
		int pipes[num_commands - 1][2];
		for (int i = 0; i < num_commands - 1; i++) {
			if (pipe(pipes[i]) < 0) {
				perror("pipe");
				exit(1);
			}
		}

		// Looking for redirection
		for (int i = 0; i < number_args; i++) {
			if (parsed_args[i] == NULL) {
				continue;
			}
			
			if (parsed_args[i+1] == NULL && (strcmp(parsed_args[i], ">") == 0 || strcmp(parsed_args[i], ">>") == 0 || strcmp(parsed_args[i], "<") == 0)) {
				fprintf(stderr, "syntax error: missing filename after %s\n", parsed_args[i]);
				has_error = 1;
				break;
			}
			if (strcmp(parsed_args[i], ">") == 0) {
				output_file = parsed_args[i+1];
				parsed_args[i] = NULL;
			} else if (strcmp(parsed_args[i], ">>") == 0) {
				output_file = parsed_args[i+1];
				is_append = 1;
				parsed_args[i] = NULL;
			} else if (strcmp(parsed_args[i], "<") == 0) {
				input_file = parsed_args[i+1];
				parsed_args[i] = NULL;
			}
		}

		if(has_error) {
			continue;
		}

		// Fork for each command
		for (int i = 0; i < num_commands; i++){
			int rc = fork();

			if (rc < 0){
				fprintf(stderr, "Fork failed...\n");
				exit(1);
			} else if (rc == 0){
				// Child process for command i
				
				// Set up pipe input (if not first command)
				if (i > 0) {
					dup2(pipes[i-1][0], 0);
				}

				// Set up pipe output (if not last command)
				if (i < num_commands - 1) {
					dup2(pipes[i][1], 1);
				}

				// Close ALL pipe file descriptors
				for (int j = 0; j < num_commands - 1; j++) {
					close(pipes[j][0]);
					close(pipes[j][1]);
				}

				// Handle input redirection
				if (input_file != NULL) {
					int fd_in = open(input_file, O_RDONLY);
					if (fd_in < 0) {
						perror("open input");
						exit(1);
					}
					dup2(fd_in, 0);
					close(fd_in);
				}

				// Handle output redirection
				if (output_file != NULL) {
					int flags = O_WRONLY | O_CREAT | (is_append ? O_APPEND : O_TRUNC);
					int fd_out = open(output_file, flags, 0644);
					if (fd_out < 0) {
						perror("open output");
						exit(1);
					}
					dup2(fd_out, 1);
					close(fd_out);
				}

				// Try builtin first
				if (execute_builtin(commands[i])) {
					exit(0);
				}

				// Execute external command
				execvp(commands[i][0], commands[i]);
				fprintf(stderr, "execvp failed\n");
				exit(1);
			}
		}

		// Parent closes all pipes
		for (int i = 0; i < num_commands - 1; i++) {
			close(pipes[i][0]);
			close(pipes[i][1]);
		}

		// Parent waits for all children
		for (int i = 0; i < num_commands; i++) {
			wait(NULL);
		}
	}
	free(parsed_args);
	return 0;
}