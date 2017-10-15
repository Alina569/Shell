#include "shell.h"
int main(int argc, char **argv){
    
    char *line = (char*) malloc(sizeof(char)*INPUT_SIZE);
    
    // do something with the history file
    int status = TRUE;
    while(status){
        
        printf(">>");
        fflush(stdout);
        parse_input(line);
        
        if (strlen(line) != 0) {
            // do something
        }
    }
    // close the history file
    return 0;
}
int parse_input(char *line){
    return 0;
}
