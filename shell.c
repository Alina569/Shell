#include "shell.h"

int main(int argc, char **argv){

	int status = TRUE;
	char *line = (char*) malloc(sizeof(char)*INPUT_SIZE);

	struct Process *commands, *tmp;

	configure();

	do {
		printf("> ");
		fflush(stdout);

		parse_input(line);
		fflush(stdout);
		if(strlen(line) != 0) {
			commands = parse_commands(line);
			status = execute_command(commands);

			while(commands != NULL) {
				tmp = commands;
				commands = commands->pipe;

				free(tmp->argv);
				free(tmp);
			}
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

	struct Process *newProcess;
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
			case '>':
			break;
			case '<':
			break;
			case '|':
				process->argc++;
				process-> argv[position] = '\0';
				newProcess = (struct Process*) malloc(sizeof(struct Process));
				newProcess->pipe = process;
				newProcess->argc = 0;
				newProcess->argv = (char**) malloc(sizeof(char) * INPUT_SIZE);
				process = newProcess;
				position = 0;
			break;
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
	switch(pid){
		case -1:
			perror("Fork exc_commands");
		break;
		case 0:
			//child -- check for fileOut and fileIn
			if (fileOut != NULL){
				// open and close the pipe
				int des_out = open(fileOut, O_WRONLY|O_CREAT, 0600);
				dup2(des_out, fileno(stdout));
				close(des_out);
			}
			// put a return in here
			// execute normaly
			return run_command(process);
		break;
		default:
			if (!background_flag) {
				do {
					waitpid(pid, &status, WUNTRACED);
				} while(!WIFEXITED(status) && !WIFSIGNALED(status));
			}
		break;
	}
	return 1;
}

int run_command(struct Process *process) {
	int file_des[2], count;
	pid_t pid;

	if(process->pipe != NULL) {

		//pipe(file_des);
		pid = fork();
		switch(pid){
			case -1:
				perror("Fork run_command");
				return -1;
			break;
			case 0:
				dup2(file_des[WRITE], WRITE);
				close(file_des[WRITE]);
				close(file_des[READ]);

				return run_command(process->pipe);
			break;
			default:
				dup2(file_des[READ], READ);
				close(file_des[READ]);
				close(file_des[WRITE]);

				int run = execvp(process->argv[0], process->argv);

				return run;
			break;
		}
	} else {
		int run = execvp(process->argv[0], process->argv);
		return run;
	}
	return 1;
}
