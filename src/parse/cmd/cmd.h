#ifndef CMD_H
#define CMD_H

#include "parse/redirect/redirect.h"

/**
 * @brief Stores raw data for a single command.
 *
 * Stores each argument as a list of characters and a list of redirects on this
 * command.
*/
typedef struct Cmd_t {
	char** argv;		/**< The arguments comprising the command. */
	int argc;			/**< The number of arguments in the command. */
	Redirect_t* redirs;	/**< List of redirects to perform on this command. */
	int redirCount;		/**< The type of the redirect. */
} Cmd_t;

void applyRedirects(Cmd_t* cmd);

#endif
