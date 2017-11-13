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

#define INPUT_SIZE 256

#define ESCAPE '\33'
#define BACKSPACE '\177'
#define ENTER '\n'
#define DELETE '~'

// Utilities -- structs

struct Process {
    int argc;
    char **argv;
    struct Process *pipe;
};

// Utilities -- globals
int background_flag;
char *fileIn, *fileOut;

struct termios termios_current;
struct termios termios_save;

// Utilities -- fucntion  headers

int parse_input(char* line);
struct Process *parse_commands(char* line);
int execute_command(struct Process *process);
int run_command(struct Process *process);

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

void configure(){

	tcgetattr(0, &termios_current);
	termios_save = termios_current;

	termios_current.c_lflag &= ~(ICANON|ECHO);
	tcsetattr(0, TCSANOW, &termios_current);
}

void recover_state(){
	tcsetattr(0, TCSANOW, &termios_save);
}

// print the history file
int print_history() {
	execlp("cat", "cat", "-n", ".history", NULL);
	return 2;
}
