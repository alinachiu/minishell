
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vect.h"
#include "tokens.h"

/* Tokenizes the STDIN input with a maximum char limit as defined in the headerfile. */
int main(int argc, char **argv) {
	char commands[MAX_CHAR];
	fgets(commands, MAX_CHAR, stdin);
	commands[strcspn(commands, "\n")] = 0;

	vect_t *tokenized_vals = tokenize(commands);

 	for(int i = 0; i < vect_size(tokenized_vals); i++) {
        	printf("%s\n", vect_get(tokenized_vals, i));
 	}

	vect_delete(tokenized_vals);
	return 0;
}