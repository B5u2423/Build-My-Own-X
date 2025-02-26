#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <string.h>

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_FAILURE
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_FAILURE
} PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef struct {
    StatementType type;
} Statement;

void insert_statement();
void select_statement();

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

MetaCommandResult do_meta_command(InputBuffer *input_buffer)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_FAILURE;
}

StatementType prepare_statement(InputBuffer *input_buffer, Statement *statement) {
    if (strncmp(input_buffer->buffer, "select", 6) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    if (strcmp(input_buffer->buffer, "insert") == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_FAILURE;
}

void execute_statement(Statement *statement)
{
    switch (statement->type)
    {
    case (STATEMENT_INSERT):
        insert_statement();
        break;
    case (STATEMENT_SELECT):
        select_statement(); 
        break;
    }
}

void insert_statement()
{
    puts("do INSERT something");
}

void select_statement()
{
    puts("do SELECT something");
}