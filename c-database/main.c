#include "repl.c"
#include <string.h>
#include <stdbool.h>

int main(int argc, char **argv) 
{
    InputBuffer *input_buffer = new_input_buffer();
    Statement statement;
    while (true) {
        print_prompt();
        read_input_buffer(input_buffer);

        if (input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
            case META_COMMAND_SUCCESS:
                exit(EXIT_SUCCESS);
            case META_COMMAND_FAILURE:
                printf("Error unrecognized command: '%s'\n", 
                    input_buffer->buffer);
                continue;
            }
        }

        switch (prepare_statement(input_buffer, &statement)) {
        case PREPARE_SUCCESS:
            break;
        case PREPARE_FAILURE:
            printf("Error unrecognized command: '%s'\n", 
                input_buffer->buffer);
            continue;;
        }

        execute_statement(&statement);
        continue;
    }

}