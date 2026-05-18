#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parse/token/token.h"
#include "parse/separator/separator.h"
#include "exec/exec.h"


int main() {

	// Main loop
	while (1) {

		// Retrieve current working directory
	    char *cwd = getcwd(NULL, 0);
		if (cwd == NULL) {
		    perror("cwd");
		    printf("st3sh: ? ➜ ");
		} else {
		    printf("st3sh: %s ➜ ", cwd);
		    free(cwd);
		}
		fflush(stdout);

		// Get user input
		char *line = NULL;
		size_t buffer = 0;

		if (getline(&line, &buffer, stdin) == -1) {
			if (feof(stdin)) {
				printf("\n");
				break;
			} else {
	    		perror("getline");
	    		continue;
			}
		}

		// Tokenize input (first pass), just do one pass for now
		Token_t* tokens = NULL;
		int numTokens = 0;
		int status = tokenizeInput(line, &numTokens, &tokens, 0);

		// Put input into a list of separators (second pass)
		int listLen = 0;
		Separator_t* processedInput = constructCommands(tokens, numTokens, &listLen);

		// Execute all entires in command list
		for (int i = 0; i < listLen; ++i) {
			execSeparator(processedInput + i);
		}
		
		// Free resources
		freeTokens(tokens, numTokens);
		freeSeparator(processedInput, listLen);
		free(line);
	}

	return 0;
}
