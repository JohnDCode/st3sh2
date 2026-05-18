#ifndef SEPARATOR_H
#define SEPARATOR_H

#include "parse/pipeline/pipeline.h"
#include "parse/token/token.h"

/**
 * @brief Stores command data for a group of pipes that are separated by
 * operators.
*/
typedef struct Separator_t {
	Pipeline_t* pipes;		/**< The list of pipelines that are separated. */
	int pipeCount;			/**< The number of pipelines. */
	TokenType_t* operators;	/**< The operators splicing pipes together. */
} Separator_t;

Separator_t* constructCommands(Token_t* tokens, int numTokens, int* listLen);
void freeSeparator(Separator_t* sep, int sepLen);

#endif
