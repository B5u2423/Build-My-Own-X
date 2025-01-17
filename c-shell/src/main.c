#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUF_SIZE 100
#define EXEC_ENV_VAR "PATH"

// TODO - Add support for command piping.
// TODO - Implement input redirection.
// TODO - Introduce command history and navigation using arrow keys.
// TODO - Add tab autocompletion for commands and paths.
// TODO - Enhance input parsing for additional special characters (like `$`).

/**
 * Return `true` if `command` is a builtin command, `false` otherwise.
 */
bool is_builtin(const char* command);

/**
 * Traverse `PATH` and returns a pointer `output` to the full path of
 * the `command` executable.
 */
int find_command_exe(char **command, const char *paths, char **output);

/**
 * Escape backslash inside a string. Destructive.
 */
void parse_backslash (char **arg);

/**
 * Remove spaces from a string. Destructive.
 */
void remove_spaces (char **arg);

/**
 * Remove a specified char `c` from a string. Destructive.
 */
void remove_char (char **arg, char c);

/**
 * 
 */
void parse_quotes (char **arg, char delimiter);

/**
 * Check for character `c` in string. Return `true` at the first occurrence,
 * `false` character does not occur.
 */
bool check_char (char **arg, char c);

/**
 * Designated function to parse and escape the double quotes in the string.
 */
void parse_double_quotes_with_backslash (char **arg);

/**
 * Parse command and arguments from user input string that has either the double
 * quotes or single quotes.
 * 
 * @param input: The raw input string.
 * @param buf: Buffer because the function use `strtok()`, which is destructive.
 * @param command: Output - parsed command from input.
 * @param arg: Output - parse arguments from input.
 * @param token_char: Delimiter for quotes as a single character.
 * @param token_str: Delimiter for quotes as a string.
 */
void parse_command_n_args (char input[], char buf[], char **command, char **args, char token_char, char *token_str);

/**
 * Redirect `stdout` and `stderr` to a file destination `dest`. 
 * 
 * @param fileno: File descriptor number for `stdout` or `stderr`.
 * @param is_append: Default is `false`, overwrite the existing file.
 * If `true`, append new data to the existing file.
 */
void pipe2file (const char *dest, int fileno, bool is_append);

int main() {
  char *program_path = malloc(sizeof(char)*BUF_SIZE);

  while (1) {
    // Flush after every printf
    setbuf(stdout, NULL);
    memset(program_path, '\0', sizeof(char)*BUF_SIZE);
    printf("$ ");
    // Wait for user input
    char input[BUF_SIZE];
    fgets(input, BUF_SIZE, stdin);

    if (strlen(input) == 1) {
      continue;
    }

    // Strip the trailing `\n`
    input[strlen(input) - 1] = '\0';
  
    // A temp buffer because `strtok` is destructive
    char buf[BUF_SIZE];
    char dest[BUF_SIZE];
    char *command = NULL;
    char *args_raw = NULL;
    char *pipe_dest = NULL;
    bool out_pipe = false, error_pipe = false, append = false;
    char *tmp_ptr = input;
    bool custom_bin = false;

    strncpy(buf, input, BUF_SIZE);

    // Redirection. `1` and blank for `stdout`, `2`for `stderr`.
    if ((check_char(&tmp_ptr, '>'))) {
      // Correct syntax, char `>` is sandwiched between 2 blank spaces.
      // Check for leading blank space.
      pipe_dest = strrchr(tmp_ptr, '>');
      if (
        *(pipe_dest - 1) != '>' && *(pipe_dest - 1) != ' ' &&
        *(pipe_dest - 1) != '2' && *(pipe_dest - 1) != '1'
      ) {
        puts("Please separate the '>' with a space");
        continue;
      }

      // Check
      if (*(pipe_dest - 1) == '>') { // Append
        append = true;
        if (*(pipe_dest - 2) == ' ' || *(pipe_dest - 2) == '1') {
          out_pipe = true;
        } else if (*(pipe_dest - 2) == '2') {
          error_pipe = true;
        }
        *(pipe_dest - 2) = '\0';
      } else if (*(pipe_dest - 1) == ' ' || *(pipe_dest - 1) == '1') { // stdout
        out_pipe = true;
        *(pipe_dest - 1) = '\0';
      } else if (*(pipe_dest - 1) == '2') { // stderr
        error_pipe = true;
        *(pipe_dest - 1) = '\0';
      }
      // Check for trailing blank space.
      if (*(pipe_dest + 1) != ' ') {
        puts("Please separate the '>' with a space");
        continue;
      }
      strncpy(dest, pipe_dest + 2, strlen(pipe_dest + 2));
      pipe_dest = dest;
      remove_char(&pipe_dest, ' ');
    }

    // I fcking hate this
    if (input[0] == '\'') {
      parse_command_n_args(input, buf, &command, &args_raw, '\'', "'");
    } else if (input[0] == '"') {
      parse_command_n_args(input, buf, &command, &args_raw, '"', "\"");
    } else if (input[0] == '.' && input[1] == '/') { // run executable relative to working dir.
      custom_bin = true;
      memset(buf, '\0', sizeof(buf));
      char *bin_file = strchr(input, '/');

      getcwd(program_path, BUF_SIZE);
      strncat(program_path, bin_file, strlen(bin_file));

      command = bin_file + 1;
      args_raw = strchr(input, ' ');
    } else {
      command = strtok(buf, " ");
      args_raw = strchr(input, ' ');
    }

    if (args_raw != NULL) {
      args_raw = args_raw + 1; // Remove the leading space
    }

    // Eval (Holy fck this is spaghetti)
    if (strcmp(command, "exit") == 0) {
      int return_val = 0;
      if (args_raw == NULL) {
        ;
      } else if ((return_val = atoi(args_raw)) == 0 && strlen(args_raw) != 1) {
        puts("Please enter an integer. Usage: exit <integer_return_value>");
        continue;
      } 
      free(program_path);
      return return_val;
      
    } else if (strcmp(command, "cd") == 0) {
      char *dest_path = (strcmp(args_raw, "~")) ? args_raw : getenv("HOME");
      if (chdir(dest_path) != 0) {
        fprintf(stderr, "cd: %s: %s\n", args_raw, strerror(errno));
      }
    } else if (strcmp(command, "echo") == 0) {
      int saved_state = dup(STDOUT_FILENO);
      // Check for quotes of arguments.
      if (args_raw[0] == '\'') {
        parse_quotes(&args_raw, '\'');
      } else if (args_raw[0] == '"') {
        if (check_char(&args_raw, '\\')) {
          parse_double_quotes_with_backslash(&args_raw);
        } else {
          parse_quotes(&args_raw, '"');
        }
      } else {
        remove_spaces(&args_raw);
        remove_char(&args_raw, '\\');
      }
      if (out_pipe) {
        pipe2file(pipe_dest, STDOUT_FILENO, append);
      } else if (error_pipe) {
        pipe2file(pipe_dest, STDERR_FILENO, append);
      }
      printf("%s\n", args_raw);
      dup2(saved_state, STDOUT_FILENO);
      close(saved_state);
    } else if (strcmp(command, "pwd") == 0) {
      memset(buf, '\0', sizeof(buf));
      printf("%s\n", getcwd(buf, sizeof(buf)));
    } else if(strcmp(command, "type") == 0) {
      if (args_raw == NULL) {
        fprintf(stderr, "Invalid parameters");
        continue;
      }
      args_raw = strtok(args_raw, " ");
      if (is_builtin(args_raw)) {
        printf("%s is a shell builtin\n", args_raw);
        continue;
      }
      if (find_command_exe(&args_raw, getenv(EXEC_ENV_VAR), &program_path)) {
        printf("%s is %s\n", args_raw, program_path);
        continue;
      }
      printf("%s: not found\n", args_raw);
    } else {
      if (find_command_exe(&command, getenv(EXEC_ENV_VAR), &program_path) || custom_bin) {
        pid_t pid = fork();
        if (pid == 0) { // Child
          char *argv[BUF_SIZE];
          int argc = 0;
          char *filename = strrchr(program_path, '/') + 1;
          // Redirection based on flags
          if (out_pipe) {
            pipe2file(pipe_dest, STDOUT_FILENO, append);
          } else if (error_pipe) {
            pipe2file(pipe_dest, STDERR_FILENO, append);
          }

          argv[argc++] = filename;
          if (args_raw != NULL) {
            char *token_delim;
            memset(buf, '\0', sizeof(buf));
            strncpy(buf, args_raw, strlen(args_raw));
            
            switch(args_raw[0]) {
              case '\'': {
                token_delim = "'";
                break;
              }
              case '"': {
                token_delim = "\"";
                break;
              } 
              default: token_delim = " ";
            }

            char *arg = strtok(buf, token_delim);
            while (arg != NULL) {
              if (strcmp(arg, " ") != 0) {
                argv[argc++] = arg;
              }
              arg = strtok(NULL, token_delim);
            }
          }
          argv[argc] = NULL;

          if (execv(program_path, argv)) {
            perror("execv() failed");
            return 1;
          }
        } else if (pid > 0) { // Parent
          wait(NULL);
        } else {
          perror("fork() failed");
          return 1;
        }
        continue;
      }
      printf("%s: command not found\n", command);
    }
  }
}

bool is_builtin(const char* command) {
  if (
    !strcmp(command, "echo") || !strcmp(command, "type") || 
    !strcmp(command, "exit") || !strcmp(command, "pwd")  ||
    !strcmp(command, "cd")
  ) {
    return true;
  }
  return false;
}

int find_command_exe(char **command, const char *paths, char **output) {
  char *m_paths = malloc(strlen(paths));
  strncpy(m_paths, paths, strlen(paths));
  char *token = strtok(m_paths, ":");

  while (token != NULL) {
    DIR *dir_ptr = opendir(token);

    // Carry on if counter any error with opening the dir
    if (dir_ptr == NULL) {
      token = strtok(NULL, ":");
      continue;
    }

    struct dirent *entry;
    while ((entry = readdir(dir_ptr)) != NULL) {
      if (strcmp(entry->d_name, *command) == 0) {
        // Make the full path
        strncpy(*output, token, strlen(token));
        strcat(*output, "/");
        strncat(*output, *command, strlen(*command));
        closedir(dir_ptr);
        free(m_paths);
        return 1;
      }
    }
    closedir(dir_ptr);
    token = strtok(NULL, ":");
  }
  free(m_paths);
  return 0;
}

void parse_quotes (char **arg, char delimiter) {
  bool need_space = false, in_quote = false;
  int write_index = 0;

  for (int i = 0; (*arg)[i] != '\0'; i++) {
    char current = (*arg)[i];
    if ((*arg)[i] == delimiter) {
      if (!in_quote && need_space) {
        (*arg)[write_index++] = ' ';
        need_space = !need_space;
      }
      in_quote = !in_quote;
      (*arg)[write_index++] = current;
    } else if (in_quote) {
      (*arg)[write_index++] = current;
    } else {
      if (current == ' ' && (i != 0 && (*arg)[i - 1] == delimiter)) {
        need_space = true;
      }
    }
  }
  (*arg)[write_index++] = '\0';
  remove_char(arg, delimiter);
}

void parse_backslash (char **arg) {
  int len = strlen(*arg);
  int i = 0, j = 0;

  while (i < len) {
    if ((*arg)[i] == '\\') {
      if ((*arg)[i - 1] == '\\') {
        (*arg)[j++] = (*arg)[i++];
        continue;
      }
      i++;
    } else {
      (*arg)[j++] = (*arg)[i++];
    }
  }
  (*arg)[j] = '\0';
}

void parse_double_quotes_with_backslash (char **arg) {
  int write_index = 0;
  bool retained = false;

  for (int i = 0; (*arg)[i] != '\0'; i++) {
    char current = (*arg)[i];
    if ((*arg)[i] == '\\') {
      if (retained) { // The string has double backslash, keep the second one.
        (*arg)[write_index++] = current; 
      }
      retained = !retained;
    } else if ((*arg)[i] == '"') {
      if (retained) { // Backslash appears before this char, so we keep it.
        retained = !retained;
        (*arg)[write_index++] = current; 
        continue;
      }
    } else {
      if (retained) retained = false;
      (*arg)[write_index++] = current; 
    }
  }
  (*arg)[write_index++] = '\0'; 
}

void remove_spaces (char **arg) {
  int len = strlen(*arg);
  int i = 0, j = 0;

  while (i < len) {
    if ((*arg)[i] == ' ' && ((*arg)[i-1] == ' ' || i == 0)) {
      i++;
    } else {
      (*arg)[j++] = (*arg)[i++];
    }
  }
  (*arg)[j] = '\0';
}

void remove_char (char **arg, char c) {
  int len = strlen(*arg);
  int i = 0, j = 0;

  while (i < len) {
    if ((*arg)[i] == c) {
      i++;
    } else {
      (*arg)[j++] = (*arg)[i++];
    }
  }
  (*arg)[j] = '\0';
}

bool check_char (char **arg, char c) {
  int len = strlen(*arg);
  for (int i = 0; i < len; i++) {
    if ((*arg)[i] == c) return true;
  }
  return false;
}

void parse_command_n_args (char input[], char buf[], char **command, char **args, char token_char, char *token_str) {
  *command = strtok(buf, token_str);
  *args = strchr(input, token_char);
  *args = strchr(*args + 1, token_char);
  *args = strchr(*args, ' ');  
}

void pipe2file (const char *dest, int fileno, bool is_append) {
  if (fileno != STDERR_FILENO && fileno != STDOUT_FILENO) {
    fprintf(stderr, "Invalid file descriptor number");
    exit(1);
  }
  int flg = is_append ? O_APPEND : O_TRUNC;
  int output_fd = open(dest, O_WRONLY | O_CREAT | flg, 0664);
  if (dup2(output_fd, fileno) == -1) {
    perror("dup2() error");
  }
  close(output_fd);
}