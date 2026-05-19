#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parse/token/token.h"
#include "parse/separator/separator.h"
#include "exec/exec.h"

int main() {

	// Status of last executed cmd
	int exitStatus = 0;

	// Main loop
	while (1) {

		// Print proc cwd and exit code of last cmd
	    char *cwd = getcwd(NULL, 0);
		if (cwd == NULL) {
		    perror("cwd");
		    if (exitStatus) { printf("(%d) ", exitStatus); }
		    printf("st3sh: ? > ");
		} else {
			if (exitStatus) { printf("(%d) ", exitStatus); }
		    printf("st3sh: %s > ", cwd);
		    free(cwd);
		}
		fflush(stdout);

		// Get user input
		char* line = NULL;
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

		// Tokenize input (first pass)
		Token_t* tokens = NULL;
		int numTokens = 0;
		int tokenStatus = tokenizeInput(line, &numTokens, &tokens, 0);

		// Keep querying user input until all pipes/quotes/seps are closed
		while (tokenStatus != 0) {

			// Get user input again
			line = NULL;
			buffer = 0;
			printf("> ");
			if (getline(&line, &buffer, stdin) == -1) {
				if (feof(stdin)) {
					printf("\n");
					break;
				} else {
		    		perror("getline");
		    		continue;
				}
			}

			// Tokenize again
			tokenStatus = tokenizeInput(line, &numTokens, &tokens, tokenStatus);
		}

		// Put input into a list of separators (second pass)
		int listLen = 0;
		Separator_t* processedInput = constructCommands(tokens, numTokens, &listLen);

		// Execute all entires in command list
		for (int i = 0; i < listLen; ++i) {
			exitStatus = execSeparator(processedInput + i);
		}
		
		// Free resources
		freeTokens(tokens, numTokens);
		freeSeparator(processedInput, listLen);
		free(line);
	}

	return 0;
}
