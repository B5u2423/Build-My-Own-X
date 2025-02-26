#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <aio.h>

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

InputBuffer *new_input_buffer()
{
    InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void close_input_buffer(InputBuffer *input_buffer) 
{
    free(input_buffer->buffer);
    free(input_buffer);
}

void read_input_buffer(InputBuffer *input_buffer)
{
    ssize_t bytes_read = getline(
        &(input_buffer->buffer),
        &(input_buffer->buffer_length),
        stdin
    );

    if (bytes_read <= 0) {
        puts("Error reading input");
        exit(EXIT_FAILURE);
    }

    // Ignore trailing newline
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[input_buffer->input_length] = 0;
}

void print_prompt() { printf("mysqlite > "); }

