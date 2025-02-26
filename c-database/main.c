#include "repl.c"
#include <string.h>
#include <stdbool.h>

int main(int argc, char **argv) 
{
    InputBuffer *input_buffer = new_input_buffer();
    while (true) {
        print_prompt();
        read_input_buffer(input_buffer);

        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
            exit(EXIT_FAILURE);
        }
    }

}