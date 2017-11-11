#include "shell.h"

int main(int argc, char **argv){

	int status = TRUE;
	char *line = (char*) malloc(sizeof(char)*INPUT_SIZE);

	struct Process *commands;

	configure();

	do {
		printf("> ");
		fflush(stdout);

		parse_input(line);
		fflush(stdout);
		if(strlen(line) != 0) {
			commands = parse_commands(line);
			status = execute_command(commands);
		}
	} while(status);

	recover_state();
	return 0;
}


int parse_input(char *line) {

	char buffer;
	int position = 0;

	// int history_nav = history_count;

	while(read(0, &buffer, 1) >= 0) {
		if (buffer > 0) {
			switch (buffer) {
				case ESCAPE:
					read(0, &buffer, 1);
					read(0, &buffer, 1);

					printf("Arrow key");
				break;
				case BACKSPACE:
					if (position >= 1) {
						write(2, "\10\33[1P", 5);
						position--;
					}
				break;
				case DELETE:
					write(2, "\33[1P", 4);
				break;
				case ENTER:
					line[position] = '\0';
					printf("\n");
					fflush(stdout);
					return 1;
				break;
				default:
					line[position] = buffer;
					position++;
					write(2, &buffer, 1);
				break;
			}
		}
	}
	return 0;
};

struct Process *parse_commands(char* line) {
	char *token;
	int position, str_init;

	struct Process *newProc;
	struct Process *process = (struct Process*) malloc(sizeof(struct Process));

	process->argc = 0;
	process->argv = (char**) malloc(INPUT_SIZE * sizeof(char));

	// global config
	background_flag = FALSE;
	fileOut = NULL;
	fileIn = NULL;

	// local config
	str_init = 0;
	position = 0;

	token = strtok(line, " ");
	while(token != NULL){
		switch(token[0]){
			// add | > < & and quotes
			case '&':
				background_flag = TRUE;
			break;
			default:
				process->argc++;
				process->argv[position] = token;

				if (str_init) {
					token = strtok(NULL, " ");
					while(token != NULL){
						process->argv[position] = concat(process->argv[position], token);

						if (token[strlen(token)-1] == '\"' || token[strlen(token)-1] == '\'') {
							break;
						}
						token = strtok(NULL, " ");
					}
					process->argv[position]++;
					process->argv[position][strlen(process->argv[position])-1] = '\0';
				}
				position++;
			break;
		}
		token = strtok(NULL, " ");
	}
	process->argv[position] = '\0';
	return process;
}

int execute_command(struct Process *process) {
	pid_t pid;

	int status;

	pid = fork();
	if (pid == 0){
		status = execvp(process->argv[0], process->argv);
		return status;
	}
	return -1;
}

