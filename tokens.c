/**
 * A simple tokenizer for a basic Unix shell.
 */

#include <stdio.h>
#include "vect.h"
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

/**
 * The function will return a non-zero value if the given character is a
 * special character, otherwise it will return 0.
 * A special character is one of: parentheses, input redirection,
 * output redirection, sequencing, pipe, or quotes.
 */
static int special_char(char ch) {
  return ch == '<' || ch == '>' || ch == ';' || ch == '(' || ch == ')' || ch == '|';
}

/**
 * Add char input to vector if the given input is valid (length is not 0).
 * Resets char input if necessary (if the token is added, a new input
 * will begin).
 */
static void add_and_reset(vect_t *vector, char *ch) {
  if (strlen(ch) != 0) {
    vect_add(vector, ch);
    memset(ch, '\0', MAX_CHAR);
  }
}

/**
 * Runs under the assumption that the given string starts with a "\""
 * symbol. Loops through the string from given index until the closing quote is found.
 * If no quote is found, then exit with a status of 1 because this is
 * unexpected behavior. If a quote is found, add the entire string
 * to the given vector and reset the given string to be an exit character
 * "/0". Returns new updated index.
 */
static int handle_quotes(vect_t* vect, char* curr_string, char *result, int i) {
  while (curr_string[i + 1] != '\0') {
    if (curr_string[i + 1] == '\0') {
      exit(1);
    } else if (curr_string[i + 1] != '\"') {
      strncat(result, &curr_string[i + 1], sizeof(curr_string[i + 1]));
      i++;
    } else {
      break;
    }
  }

  return i + 2;
}

/**
 * Tokenize a given string input. Returns a vector with substrings
 * from the given string, based on the existence of whitespace,
 * tabs, or special tokens which are defined in the special char method.
 */
vect_t *tokenize(char *curr_string) {
  vect_t *vector = vect_new(); // vector with all tokens
  char result[MAX_CHAR] = "\0"; // represents current result token
  int i = 0; // index to keep track of what part of the string we are in

  while (i < MAX_CHAR && curr_string[i] != '\0') {
    if ((strncmp(&curr_string[i], " ", 1) == 0)) {
      // if space, add whatever we were currently building and reset current result token
      add_and_reset(vector, result);
      i++;
    } else if (strncmp(&curr_string[i], "\\", 1) == 0
		    && ((i + 1) < MAX_CHAR)
		    && (curr_string[i + 1] != '\0')
		    && (strncmp(&curr_string[i + 1], "t", 1) == 0)) {
      // if tab, add whatever we were currently building and reset current result token
      add_and_reset(vector, result);
      i += 2;
    } else if (curr_string[i] == '\"') {
      // if quote, try to find the closing quote and tokenize if possible
      add_and_reset(vector, result);
      i = handle_quotes(vector, curr_string, result, i);
    } else if (special_char(curr_string[i]) == 1) {
      // check if current char is a special token character
      add_and_reset(vector, result);
      // add the special symbol to the result
      strncat(result, &curr_string[i], sizeof(curr_string[i]));
      add_and_reset(vector, result);
      i++;
    } else {
      // if it's none of the above, append to current string since it's not special
      strncat(result, &curr_string[i], sizeof(curr_string[i]));
      i++;
    }
  }

  // add the final token to the vector, then return all tokenized values in a vector
  add_and_reset(vector, result);
  return vector;
}