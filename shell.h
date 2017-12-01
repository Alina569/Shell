// Utilities -- includes

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

// Utilities -- defines

#define WRITE 1
#define TRUE 1
#define READ 0
#define FALSE 0
#define ERROR 2

#define INPUT_SIZE 256

#define ESCAPE '\33'
#define BACKSPACE '\177'
#define ENTER '\n'
#define DELETE '~'
#define NONE '\0'

// Utilities -- structs

struct Process {
    int argc;
    char **argv;
    struct Process *pipe;
};

// Utilities -- globals
int background_flag;
char *fileIn, *fileOut;
FILE *history_file;

// hist -- utilities
int history_count = 0;
char history[256][INPUT_SIZE];

struct termios termios_current;
struct termios termios_save;

// Utilities -- fucntion  headers

int parse_input(char* line);
struct Process *parse_commands(char* line);
int execute_command(struct Process *process);
int run_command(struct Process *process);
int cmp_exc_command(char **argv);

// Utilities -- functions

// stackoverflow answer.
char* concat(const char *string1, const char *string2) {
	const size_t len_str1 = strlen(string1);
	const size_t len_str2 = strlen(string2);

	char *result = malloc(len_str1 + len_str2 + 1); // NULL end
	memcpy(result, string1, len_str1);
	memcpy(result + len_str1, string2, len_str2 + 1); // cpy NULL

	return result;
}

// Terminal state -- functions

void configure(){

	tcgetattr(0, &termios_current);
	termios_save = termios_current;

	termios_current.c_lflag &= ~(ICANON|ECHO);
	tcsetattr(0, TCSANOW, &termios_current);
}

void recover_state(){
	tcsetattr(0, TCSANOW, &termios_save);
}

// History file -- functions

int print_history() {
	execlp("cat", "cat", "-n", ".tmphistory", NULL);
	return 2;
}

char* get_history(int position){
	position--;
	if (position > history_count || position < 0){
		printf("Index error");
		fflush(stdout);
		exit(1);
	} else {
		return history[position];
	}
}

void read_history(FILE *history_file) {
	rewind(history_file); // begining of file
	history_count = 0; // change to 0

	while(fgets(history[history_count], INPUT_SIZE, history_file)){
		history_count++;
	}
}

// change directory -- function

