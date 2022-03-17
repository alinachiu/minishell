/**
 * Simple Unix Shell in C by Alina Chiu.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

#include "shell.h"

/**
 * Determines the size of the given tokens and returns it.
 */
static int size(char **tokens) {
  int i = 0;
  while (tokens[i] != NULL) {
    i++;
  }
  return i;
}

/**
 * Free the memory that was used to create the given tokenized_vals
 * vector and the given tokens.
 */
static void free_mem(char **tokens) {
  // clear memory before next command is accepted
  for (int i = 0; i < size(tokens); i++) {
    free(tokens[i]);
  }
}

/**
 * Updates the given previous token vector to have the most recent
 * command that has been run based on the given tokens. The original
 * previous tokens will be deleted, if they exist, before updating.
 */
static void update_prev_tokens(char *tokens, char *prev_tokens) {
  for (int i = 0; i < MAX_CHAR; i++) {
    prev_tokens[i] = tokens[i];
  }
}

/**
 * Handles the case where there is no directory when in child process.
 */
static void no_dir(char **tokens, char *str, char *prev_tokens) {
  if (fork() == 0) {
    if (execvp(tokens[0], tokens) < 0) {
      printf("No such file or directory\n");
      exit(0);
    }
  } else {
    wait(NULL);
    update_prev_tokens(str, prev_tokens);
  }
}

/**
 * Tokenizes a given new command into the given tokens array.
 */
static void tokenize_elements(char *new_command, char **tokens) {
  new_command[strcspn(new_command, "\n")] = 0;
  vect_t* tokenized_vals = tokenize(new_command);

  for (int i = 0; i < vect_size(tokenized_vals); i++) {
    tokens[i] = vect_get_copy(tokenized_vals, i);
  }

  tokens[vect_size(tokenized_vals)] = NULL;
  vect_delete(tokenized_vals);
}

/**
 * Handles the help command by printing out a detailed help
 * message that explains all built-in commands available in minishell.
 */
static void handle_help() {
  printf("Minishell by Alina Chiu\nBuilt-In cCmmands\n---\n\n");
  printf("usage: cd [directory_name]\nChange the current working directory of the shell. If no directory is provided, changes to Home directory.\n\n");
  printf("usage: source [filename]\nTakes a filename as an argument and processes each line of the file as a command, including built-ins. In other words, each line should be processed as if it was entered by the user at the prompt.\n\n");
  printf("usage: prev\nPrints the previous command and executes it again. If there is no previous value, nothing is executed or printed\n\n");
  printf("usage: help\nExplains all built-in commands available in this minishell\n"); 
}

/**
 * Handles the cd command. If there are two tokens, change the specified
 * directory. If there is one token, go to the current users home directory.
 * If there are more than two tokens, let the user know that their cd command
 * is invalid.
 *
 * Returns the tokens used.
 */
static void handle_cd(int tokens_size, char **tokens) {
  if (tokens_size == 2) {
    if (chdir(tokens[1]) == -1) {
      printf("-shell: cd: given directory does not exist\n");
    }
  } else if (tokens_size == 1) {
    assert(chdir(getenv("HOME")) == 0);
  } else {
    printf("-shell: cd: too many arguments.\ncd: usage: cd [directory_name]\n");
  }
}

/**
 * Checks given tokens to see if there are any instances of specials.
 * Returns the integer associated with the special character.
 * More specifically, 1 for ;, 2 for <, 3 for >, and 4 for |.
 * If there are no special tokens, returns 0.
 */
 static int has_special(char **tokens) {
  for (int i = 0; i < size(tokens); i++) {
    if (strcmp(";", tokens[i]) == 0) {
      return 1;
    } else if (strcmp("<", tokens[i]) == 0) {
      return 2;
    } else if (strcmp(">", tokens[i]) == 0) {
      return 3;
    } else if (strcmp("|", tokens[i]) == 0) {
      return 4;
    }
  }
  return 0;
}

/**
 * Splits the special token array into a left and right hand side based on where the first instance of a given special token is.
 */
static void split(char *special_token, char **tokens, char **left_tokens, char **right_tokens) {
  int i = 0;
  int length = size(tokens);
  while(strcmp(special_token, tokens[i]) != 0) {
    left_tokens[i] = tokens[i];
    i++;
  }

  int j = 0;
  i++;
  while (i < length) {
    right_tokens[j] = tokens[i];
    j++;
    i++;
  }
}

/**
 * Handle input redirection by opening a new file based on the token
 * to the right of the "<" symbol.
 */
static void handle_input_redirection(char **tokens, char **left_tokens, char **right_tokens) {
  split("<", tokens, left_tokens, right_tokens);
  if (fork() == 0) {
    close(0);
    int fd = open(right_tokens[0], O_RDONLY);
    if (execvp(left_tokens[0], left_tokens) < 0) {
      printf("Incorrect input.\n");
    }
    exit(0);
  } else {
    wait(NULL);
  }
}

/**
 * Handles output redirection by creating a new file based on the
 * token to the right of the ">" symbol.
 */
static void handle_output_redirection(char **tokens, char **left_tokens, char **right_tokens) {
  split(">", tokens, left_tokens, right_tokens);
  if (fork() == 0) {
    close(1);
    int fd = open(right_tokens[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (execvp(left_tokens[0], left_tokens) < 0) {
      printf("Incorrect input.\n");
    }
    exit(0);
  } else {
    wait(NULL);
  }
}

/**
 * Performs actions if special characters exist in the given input to
 * stdin. Continues performing actions until all special characters
 * have been processed.
 */
static void execute_special(char **tokens);

/**
 * Handles the semicolon token by splitting processes into their own commands
 * and executing each command until all semicolons have been parsed.
 */
static void handle_semicolon(char **tokens, char **left_tokens, char **right_tokens);

/**
 * Handles custom piping. Runs the command on the left hand side and the
 * command on the right hand side of a given "|" character simulatenously.
 * The standard output of the left command should be redirected to the
 * standard input of the right command.
 */
static void custom_pipe(char **tokens);

/**
 * Performs actions if special characters exist in the given input to
 * stdin. Continues performing actions until all special characters
 * have been processed.
 */
 static void execute_special(char **tokens) {
  char *left_tokens[MAX_CHAR];
  char *right_tokens[MAX_CHAR];

  switch (has_special(tokens)) {
    case 1:
      handle_semicolon(tokens, left_tokens, right_tokens);
      break;
    case 2:
      handle_input_redirection(tokens, left_tokens, right_tokens);
      break;
    case 3:
      handle_output_redirection(tokens, left_tokens, right_tokens);
      break;
    case 4:
      custom_pipe(tokens);
      break;
    default:
      if (execvp(tokens[0], tokens) < 0) {
        printf("Invalid input.\n");
        exit(0);
      }
  }
}

/**
 * Handles the semicolon token by splitting processes into their own commands
 * and executing each command until all semicolons have been parsed.
 */
static void handle_semicolon(char **tokens, char **left_tokens, char **right_tokens) {
  split(";", tokens, left_tokens, right_tokens);
  if (fork() == 0) {
    execute_special(left_tokens);
  } else {
    wait(NULL);
    if (fork() == 0) {
      execute_special(right_tokens);
    } else {
      wait(NULL);
    }
  }
}

/**
 * Determines the number of pipes that exist in a given command.
 */
static int num_pipes(char **tokens) {
  int pipe_num = 0;
  for (int i = 0; i < size(tokens); i++) {
    if (strcmp(tokens[i], "|") == 0) {
      pipe_num++;
    }
  } 
  return pipe_num;
}

/**
 * Handles custom piping. Runs the command on the left hand side and the
 * command on the right hand side of a given "|" character simulatenously.
 * The standard output of the left command should be redirected to the
 * standard input of the right command.
 */
static void custom_pipe(char **tokens) {
  char *left_tokens[MAX_CHAR];
  char *right_tokens[MAX_CHAR];
  split("|", tokens, left_tokens, right_tokens);
  if (fork() == 0) {
    int pipe_fds[2];
	pipe(pipe_fds);
    int read_fd = pipe_fds[0];
    int write_fd = pipe_fds[1];

    if (fork() == 0) {
      close(read_fd);
      close(1);
      dup(write_fd);
      execute_special(left_tokens);
  	} else {
      close(write_fd);
      close(0);
      dup(read_fd);
      execute_special(right_tokens);
      wait(NULL);
    }
  } else {
    wait(NULL);
  }
}

/**
 * Executes a command based on the given token array and updates the
 * previous token string.
 *
 * Returns a non-zero value if exit condition has been called. Else, returns
 * a zero value.
 */
static int execute(char **tokens, char *commands, char *prev_tokens);

/**
 * Handles the source command. If there is only one token, print an error
 * message for the user to view. If there is more than one command, then
 * execute the source command accordingly.
 * 
 * Returns the tokens used.
 */
static void handle_source(char **tokens) {
  if (size(tokens) == 2) {
    char* new_tokens[MAX_CHAR];
    char line[MAX_CHAR];
    char prev[MAX_CHAR];
    FILE* fd_read = fopen(tokens[1], "r");
    while (fgets(line, sizeof(line), fd_read) != NULL) {
      tokenize_elements(line, new_tokens);
      if (strcmp(new_tokens[0], "exit") == 0) {
        free_mem(new_tokens);
	break;
      }
      execute(new_tokens, line, prev);
      free_mem(new_tokens);
    }
    fclose(fd_read);
  } else {
    printf("-shell: source: filename argument required\nsource: usage: source filename\n");
  }
}

/**
 * Handles the prev command. If invalid input, print a message letting the
 * user know that it is invalid. Else, print the previous command
 * and execute it.
 */
static void handle_prev(char **tokens, char *prev_tokens) {
  if (size(tokens) == 1 && prev_tokens != NULL) {
    printf("%s\n", prev_tokens);      
    char* old_tokens[MAX_CHAR];
    tokenize_elements(prev_tokens, old_tokens);
    if (strlen(prev_tokens) == 0) {
      execute(old_tokens, prev_tokens, prev_tokens);
    }
    free_mem(old_tokens);
  } else {
    printf("-shell: prev: prev requires one argument\nusage: prev: prev\n");
  }
}

/**
 * Handles the case where there are special tokens in the execute function.
 */
static void handle_specials_exec(char **tokens) {
  if (has_special(tokens) == 4) {
    if (num_pipes(tokens) > 1) {
      execute_special(tokens);
    } else {
      custom_pipe(tokens);
    }
  } else {
    execute_special(tokens);
  }
}

/**
 * Executes a command based on the given token array and updates the
 * previous token string.
 *
 * Returns a non-zero value if exit condition has been called. Else, returns
 * a zero value.
 */
static int execute(char **tokens, char *commands, char *prev_tokens) {
  if (size(tokens) != 0) {
    if (strcmp(tokens[0], "exit") == 0) {
      free_mem(tokens);
      return 1;
    } else if (has_special(tokens) > 0) {
      update_prev_tokens(commands, prev_tokens);
      handle_specials_exec(tokens);
    } else if (strcmp(tokens[0], "cd") == 0) {
      update_prev_tokens(commands, prev_tokens);
      handle_cd(size(tokens), tokens);
    } else if (strcmp(tokens[0], "source") == 0) {
      update_prev_tokens(commands, prev_tokens);
      handle_source(tokens);
    } else if (strcmp(tokens[0], "prev") == 0) {
      handle_prev(tokens, prev_tokens);
    } else if (strcmp(tokens[0], "help") == 0) {
      update_prev_tokens(commands, prev_tokens);
      handle_help();
    } else {
      update_prev_tokens(commands, prev_tokens);
      no_dir(tokens, commands, prev_tokens);
    }
  }
  return 0;
}

/**
 * Outputs the minishell commands based on specified inputs.
 */
int main(int argc, char **argv) {
  printf("Welcome to mini-shell.\n");
  char commands[MAX_CHAR]; // current commands from stdin up to max chars
  char prev_tokens[MAX_CHAR]; // previous token, if it exists
  char* tokens[MAX_CHAR];  // tokens

  // look for inputs from STDIN while not 'exit' command or ctrl-d
  while (1) {
    printf("shell $ ");

    // if ctrl+d clicked, stop accepting commands
    if (fgets(commands, MAX_CHAR, stdin) == NULL) {
      printf("\n");
      break;
    }

    // tokenize command line inputs into vector for processing
    tokenize_elements(commands, tokens);
	
    // execute given command
    if (execute(tokens, commands, prev_tokens) == 1) {
      break;
    }
    free_mem(tokens);
  }

  // Exit program.
  printf("Bye bye.\n");
  return 0;
}