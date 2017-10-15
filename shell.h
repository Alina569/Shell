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
    char **args;
    struct Process *pipe;
};

// Utilities -- globals

// Utilities -- functions

int parse_input(char* line);
