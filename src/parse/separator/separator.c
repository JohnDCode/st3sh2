#include "parse/separator/separator.h"

#include <stdio.h>
#include <stdlib.h>

#include "parse/token/token.h"
#include "parse/pipeline/pipeline.h"

/**< Buffer size for num of arguments within a command. */
static const int COMMAND_ARGUMENT_BUFFER_SIZE = 64;

/*
 * @brief Second pass for user input shell parsing.
 *
 * Parses tokens into a series of pipeline, command, and redirect objects.
 * Also handles and processes all inline expansion and escape chars.
 *
 * @param[in] tokens	The tokens array to parse.
 * @param[in] numTokens	The number of tokens to parse.
 * @param[out] listLen	The number of separators there are in the final list.
 *
 * @return Pointer to parsed separator object, NULL if error.	
 * @note Note to self: John this code is so bad clean this up later please.
*/
Separator_t* constructCommands(Token_t* tokens, int numTokens, int* listLen) {

	// Ensure there is something to process
	if (tokens == NULL || numTokens == 0) {
		*listLen = 0;
		return NULL;
	}

	// Create inital separator object to return
	Separator_t* res = malloc(sizeof(Separator_t));
	if (res == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	*listLen = 0;

	// Setup res
	res->pipeCount = 0;
	res->operators = NULL;
	res->pipes = malloc(sizeof(Pipeline_t));
	if (res->pipes == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// Setup first pipe
	res->pipes->cmds = malloc(sizeof(Cmd_t));
	res->pipes->cmdCount = 0;
	if (res->pipes->cmds == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// Setup first command within first pipe
	res->pipes->cmds->argv = \
		malloc(COMMAND_ARGUMENT_BUFFER_SIZE * sizeof(char *));
	res->pipes->cmds->argc = 0;
	res->pipes->cmds->redirs = NULL;
	res->pipes->cmds->redirCount = 0;
	if (res->pipes->cmds->argv == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// Loop over all tokens
	int i = 0;
	while (i < numTokens) {

		// Get pointer to cur token
		Token_t* curToken = tokens + i;

		// Redirect token
		if (isRedirect(curToken)) {

			// Get pointer to next token
			Token_t* nextToken = NULL;
			if (i + 1 < numTokens) {
				nextToken = curToken + 1;
			}

			// Construct the redirect object from the pair of tokens
			Redirect_t* redir = createRedirect(curToken, nextToken);
			if (redir == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			// If we set the target then step over it
			if (redir->target != NULL) {
				++i;
			}

			// Get current cmd to insert into
			Cmd_t* curCmd = ((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmds + \
				((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount;

			// Reallocate to make room in list of redirects
			curCmd->redirs = realloc(curCmd->redirs, \
				(curCmd->redirCount + 1) * sizeof(Redirect_t));
			if (curCmd->redirs == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}

			// Insert the redirect into the current command
			*(curCmd->redirs + curCmd->redirCount) = *redir;
			++(curCmd->redirCount);
			free(redir); // free temp copy

		// Pipe token
		} else if (curToken->type == TOK_PIPE) {

			/*

				Note: parse pass 1 (tokenizer) ensures that there is something
				after pipe, not that there is something before. If there is not
				something after, user is prompted again for input to "close"
				pipe. If nothing before, input is invalid and error is thrown.

			*/

			// Get current command (that was just closed by this pipe token)
			Cmd_t* curCmd = ((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmds + \
				((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount;


			// Ensure cur command has non-zero num args
			if (curCmd->argc == 0) {
				freeSeparator(res, *listLen + 1);
				return NULL;
			}

			// Realloc space in last cmd for exact number of arguments + NULL
			curCmd->argv = realloc(curCmd->argv, \
				(curCmd->argc + 1) * sizeof(char *));
			if (curCmd->argv == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
			*(curCmd->argv + curCmd->argc) = NULL;

			// Pipe marks close of a cmd, increase count accordingly
			++(((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount);

			// Allocate space for next command
			((res + *listLen)->pipes + (res + *listLen)->pipeCount)->cmds = \
				realloc(((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmds, \
				(((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount + 1) \
				* sizeof(Cmd_t));

			if (((res + *listLen)->pipes + ((res + *listLen)) \
				->pipeCount)->cmds == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}

			// Setup next command
			Cmd_t* nextCmd = ((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmds + \
				((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount;

			nextCmd->argv = \
				malloc(COMMAND_ARGUMENT_BUFFER_SIZE * sizeof(char *));
			if (nextCmd->argv == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			nextCmd->argc = 0;
			nextCmd->redirs = NULL;
			nextCmd->redirCount = 0;
			
		// Separator token
		} else if (isSeparator(curToken)) {

			/*

				Note: Similar check here as for pipes, except the most recent
				pipe that is closed here, and the most recent command that is
				closed here, both can't be empty.

			*/

			// Get current command (that was just closed by this pipe token)
			Cmd_t* curCmd = ((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmds + \
				((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount;


			// Ensure cur command/pipe has non-zero num args/cmds
			if (curCmd->argc == 0) {
				freeSeparator(res, *listLen + 1);
				return NULL;
			}

			// Realloc space in last cmd for exact number of arguments + NULL
			curCmd->argv = realloc(curCmd->argv, \
				(curCmd->argc + 1) * sizeof(char *));
			if (curCmd->argv == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
			*(curCmd->argv + curCmd->argc) = NULL;

			// Separator marks close of a cmd, increase count accordingly
			++(((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount);

			// Separator marks close of a pipe, increase count accordingly
			++(res + *listLen)->pipeCount;

			// Allocate space for new pipe
			(res + *listLen)->pipes = realloc((res + *listLen)->pipes, \
				sizeof(Pipeline_t) * ((res + *listLen)->pipeCount + 1));
			if ((res + *listLen)->pipes == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
			((res + *listLen)->pipes + (res + *listLen)->pipeCount)->cmdCount = 0;


			// Get the new pipe (that was just allocated for)
			Pipeline_t* newPipe = ((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount);

			// Setup first command in new pipeline
			newPipe->cmds = malloc(sizeof(Cmd_t));
			if (newPipe->cmds == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			newPipe->cmds->argv = \
					malloc(COMMAND_ARGUMENT_BUFFER_SIZE * sizeof(char *));
			newPipe->cmds->argc = 0;
			newPipe->cmds->redirs = NULL;
			newPipe->cmds->redirCount = 0;
			if (newPipe->cmds->argv == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			// Record operator, num of separator operators is just num of pipes
			(res + *listLen)->operators = realloc((res + *listLen)->operators,
				sizeof(TokenType_t) * ((res + *listLen)->pipeCount));
			if ((res + *listLen)->operators == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
			*((res + *listLen)->operators + ((res + *listLen)->pipeCount - 1)) \
				= curToken->type;


		// Break in command list
		} else if (curToken->type == TOK_SEMI) {

			// Similar to  &&/||

			// Get current command (that was just closed by this sep token)
			Cmd_t* curCmd = ((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmds + \
				((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount;

			// Realloc space in last cmd for exact number of arguments + NULL
			curCmd->argv = realloc(curCmd->argv, \
				(curCmd->argc + 1) * sizeof(char *));
			if (curCmd->argv == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
			*(curCmd->argv + curCmd->argc) = NULL;

			// Semi marks close of a cmd, increase count accordingly
			++(((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount);

			// Semi marks close of a pipe, increase count accordingly
			++(res + *listLen)->pipeCount;

			// Allocate space for new entry in return list
			++(*listLen);
			res = realloc(res, sizeof(Separator_t) * (*listLen + 1));
			if (res == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}

			// Setup new sep entry in res list
			(res + *listLen)->pipeCount = 0;
			(res + *listLen)->operators = NULL;
			(res + *listLen)->pipes = malloc(sizeof(Pipeline_t));
			if ((res + *listLen)->pipes == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}
			((res + *listLen)->pipes + (res + *listLen)->pipeCount)->cmdCount = 0;


			// Get the new pipe (that was just allocated for)
			Pipeline_t* newPipe = ((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount);

			// Setup first command in new pipeline
			newPipe->cmds = malloc(sizeof(Cmd_t));
			if (newPipe->cmds == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			newPipe->cmds->argv = \
					malloc(COMMAND_ARGUMENT_BUFFER_SIZE * sizeof(char *));
			newPipe->cmds->argc = 0;
			newPipe->cmds->redirs = NULL;
			newPipe->cmds->redirCount = 0;
			if (newPipe->cmds->argv == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}

		// Normal word
		} else {

			// Get raw data
			char* wordChars = extractChars(curToken, 1);
			if (wordChars == NULL) {
				free(wordChars);
				freeSeparator(res, *listLen + 1);
				return NULL;
			}

			// Get current cmd to insert into
			Cmd_t* curCmd = ((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmds + \
				((res + *listLen)->pipes + \
				(res + *listLen)->pipeCount)->cmdCount;

			// Ensure space
			if (curCmd->argc == COMMAND_ARGUMENT_BUFFER_SIZE) {
				free(wordChars);
				freeSeparator(res, *listLen + 1);
				return NULL;
			}

			// Add the argument to curCmd
			*(curCmd->argv + curCmd->argc) = wordChars;
			++curCmd->argc;
		}

		// Step to next token
		++i;
	}

	// Reallocate space in final cmd for exact number of arguments
	Cmd_t* curCmd = ((res + *listLen)->pipes + \
		(res + *listLen)->pipeCount)->cmds + \
		((res + *listLen)->pipes + \
		(res + *listLen)->pipeCount)->cmdCount;
	curCmd->argv = realloc(curCmd->argv, (curCmd->argc + 1) * sizeof(char *));
	if (curCmd->argv == NULL) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	*(curCmd->argv + curCmd->argc) = NULL;

	// Marks close of a cmd, pipe, and entry in final list, increase counts
	++(((res + *listLen)->pipes + (res + *listLen)->pipeCount)->cmdCount);
	++(res + *listLen)->pipeCount;
	++(*listLen);

	// Return pointer to constructed list of pipelines
	return res;
}

/*
 * @brief Frees all memory from a list of separators.
 *
 * Frees all memory from all separators in a list, including redirects,
 * pipelines, commands, arguments, etc.
 * 
 * @param[in] sep		The list of separators to free.
 * @param[in] sepLen	The number of separators in the list to free.
*/
void freeSeparator(Separator_t* sep, int sepLen) {

	for (int i = 0; i < sepLen; ++i) {
		for (int j = 0; j < sep[i].pipeCount; ++j) {
			for (int k = 0; k < sep[i].pipes[j].cmdCount; ++k) {

				// Free arguments for this cmd
				for (int l = 0; l < sep[i].pipes[j].cmds[k].argc; ++l) {
					free(sep[i].pipes[j].cmds[k].argv[l]);
				}
				free(sep[i].pipes[j].cmds[k].argv);

				// Free redirects for this cmd
				free(sep[i].pipes[j].cmds[k].redirs);
			}

			// Free cmsd for this pipeline
			free(sep[i].pipes[j].cmds);
		}

		// Free pipes and sep operations for this separator
		free(sep[i].pipes);
		free(sep[i].operators);
	}

	// Free all separators
	free(sep);
}
