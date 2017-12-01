#include "shell.h"

int main(int argc, char **argv){

	int status = TRUE;
	char *line = (char*) malloc(sizeof(char)*INPUT_SIZE);

	struct Process *commands, *tmp;

	configure();
	history_file = fopen(".tmphistory", "a+"); // append flag
	read_history(history_file);

	do {
		printf("> ");
		fflush(stdout);

		parse_input(line);
		fflush(stdout);

		if(strlen(line) != 0) {
			// save the command in the history file
			fprintf(history_file, "%s\n", line);
			fflush(history_file);

			read_history(history_file);
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

	fclose(history_file);
	recover_state();
	return 0;
}


int parse_input(char *line) {

	char buffer;
	int position = 0;

	int cur_hist_nav = history_count;

	while(read(READ, &buffer, 1) >= 0) {
		if (buffer > 0) {
			switch (buffer) {
				case ESCAPE:
					read(READ, &buffer, 1);
					read(READ, &buffer, 1);

					if (buffer == 'A') {
						if (cur_hist_nav-1 >= 0){
							cur_hist_nav--;

							while(position >= 1){
								write(2, "\10\33[1P", 5);
								position--;
							}
							int i; // index
							for (i=0; history[cur_hist_nav][i] != '\n'; i++) {
								position = i;
								line[i] = history[cur_hist_nav][i];
								write(ERROR, &line[i], 1);
							}
							position++;
						}

					} else if (buffer == 'B') {
						while(position >= 1) {
							write(ERROR, "\10\33[1P", 5);
							position--;
						}
						if (cur_hist_nav+1 < history_count) {
							cur_hist_nav++;

							int i;
							for(i=0; history[cur_hist_nav][i] != '\n'; i++) {
								position = i;
								line[i] = history[cur_hist_nav][i];
								write(ERROR, &line[i], 1);
							}
							position++;

						} else if (cur_hist_nav+1 == history_count){
							cur_hist_nav = history_count;
						}
					}
				break;
				case BACKSPACE:
					if (position >= 1) {
						write(ERROR, "\10\33[1P", 5);
						position--;
					}
				break;
				case DELETE:
					write(ERROR, "\33[1P", 4);
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
					write(ERROR, &buffer, 1);
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
	str_init = FALSE;
	position = 0;

	token = strtok(line, " ");
	while(token != NULL){
		switch(token[0]){
			// add | > < & and quotes
			case '>':
				process->argc++;
				process->argv[position] = NONE;
				fileOut = strtok(NULL, " ");
			break;
			case '<':
				process->argc++;
				process->argv[position] = NONE;
				fileIn = strtok(NULL, " ");
			break;
			case '|':
				process->argc++;
				process-> argv[position] = NONE;
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
			case '\"':
			case '\'':
				str_init = TRUE;
				// the brak is inside the default case. cant break here.
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
	process->argv[position] = NONE;
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

		pipe(file_des);
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

				int run = cmp_exc_command( process->argv);
				return run;
			break;
		}
	} else {
		int run = cmp_exc_command(process->argv);
		return run;
	}
	return 1;
}

int cmp_exc_command(char **argv){
	int exc_status;

	if (strcmp(argv[0], "cd") == 0) {
		if (argv[1] == NULL){
			fprintf(stderr, "Expected argument\n");
			return 1;
		}
		int cd_status = chdir(argv[1]);
		if (cd_status != 0){
			perror("chdir error");
		}
		return 1;

	} else {
		if (strcmp(argv[0], "history") == 0) {
			int print_status = print_history();
			return print_status;
		} else {
			if (fileIn != NULL){
				int file_in_desc = open(fileIn, O_RDONLY);
				dup2(file_in_desc, fileno(stdin));
				close(file_in_desc);
			}
			exc_status = execvp(argv[0], argv);
			if (exc_status == -1) {
				printf("%s: Command not found\n", argv[0]);
				fflush(stdout);
				return exc_status;
			}
			return exc_status;
		}
	}
}
