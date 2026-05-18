#include "parse/redirect/redirect.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "parse/token/token.h"

/**
 * @brief Constructs and returns a new \ref Redirect_t object.
 *
 * @param[in] token			The token op object to consturct the redirect from.
 * @param[in] targetToken	The next token in the sequence representing target.
 * 
 * @return Pointer to constructed \ref Redirect_t object.
 * 
 * Dynamically allocates a new \ref Redirect_t object with a target and
 * operator. Returns pointer to the constructed object. Extracts data from the
 * characters within two consecutivley tokens and sets all fields accordingly.
*/
Redirect_t* createRedirect(Token_t* token, Token_t* targetToken) {

	// Attempt to allocate memory
	Redirect_t* redir = malloc(sizeof(Redirect_t));
	if (redir == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// If redirect is type DUP_IN/OUT, CLOSE_IN/OUT then no target
	if (token->type >= TOK_REDIR_DUP_IN \
		&& token->type <= TOK_REDIR_DUP_CLOSE_OUT) {

		// Set targe to null
		redir->target = NULL;

	// Set target
	} else {

		// Ensure target exists
		if (targetToken == NULL || targetToken->type != TOK_WORD) {
			free(redir);
			errno = EINVAL;
			return NULL;
		}

		// Get raw data for redirect target
		char* targetChars = extractChars(targetToken, 1);
		if (targetChars == NULL) {
			free(redir);
			return NULL;
		}

		// Set
		redir->target = targetChars;
	}

	// Set type based on current token
	redir->type = token->type;

	// Extract charcters from redirect token
	char* redirChars = extractChars(token, 0);

	// Set defaults for file descriptors
	redir->fd = -1;
	redir->targetFd = -1;


	// Extract file descriptors for the redirect based on type
	if (token->type == TOK_REDIR_IN || token->type == TOK_REDIR_OUT) {

		// Extract fd
		if (strlen(redirChars) == 2) {
			redir->fd = *redirChars - '0';
		}

	} else if (token->type == TOK_REDIR_APPEND \
		|| token->type == TOK_REDIR_RW) {

		// Extract fd
		if (strlen(redirChars) == 3) {
			redir->fd = *redirChars - '0';
		}

	// Must be dup_in/out or close_in/out
	} else if (token->type != TOK_REDIR_HERE) {

		// Extract fd
		redir->fd = *redirChars - '0';

		// Extract target fd
		if (token->type == TOK_REDIR_DUP_IN \
			|| token->type == TOK_REDIR_DUP_OUT) {
			redir->targetFd = *(redirChars + 3) - '0';
		}

	}

	// Free the extracted characters from the cur token
	free(redirChars);

	// Ensure values set are valid
	if (redir->fd < -1 || redir->targetFd < -1) {
		free(redir->target);
		free(redir);
		errno = EINVAL;
		return NULL;
	}

	// Return constructed object
	return redir;
}
