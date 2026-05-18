#ifndef REDIRECT_H
#define REDIRECT_H

#include "parse/token/token.h"

/**
 * @brief Stores the redirect data for a command.
 *
 * Stores the target of the redirect, and the type of redirect to apply.
*/
typedef struct Redirect_t {
	char* target;		/**< The target of the redirect. */
	TokenType_t type;	/**< The type of redirect operator. */
	int fd;				/**< File descriptor from left side of operator. */
	int targetFd;		/**< File descriptor from right side of operator. */
} Redirect_t;

Redirect_t* createRedirect(Token_t* token, Token_t* targetToken);

#endif
