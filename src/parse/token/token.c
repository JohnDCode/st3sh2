#include "parse/token/token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>

/**< Buffer size when processing command tokens, max num tokens from input. */
static const int TOKEN_BUFFER_SIZE = 16;

/**< Buffer incremental size for storing characters within tokens. */
static const int TOKEN_CHARACTER_BUFFER_SIZE = 64;

/*
 * Stores regular expressions to match to each type of redirect token.
 * Each index in the array matches to entry in enum \ref TokenType_t.
*/
static const char* redirectRegexs[TOK_AND] = {
	"", "", // TOK_WORD and TOK_PIPE dummy slots
	"^[0-9]*<$",		// TOK_REDIR_IN
	"^[0-9]*>$",		// TOK_REDIR_OUT
	"^[0-9]*>>",		// TOK_REDIR_APPEND
	"<<",				// TOK_REDIR_HERE
	"^[0-9]*<>",		// TOK_REDIR_RW
	"^[0-9]+<&[0-9]+$",	// TOK_REDIR_DUP_IN
	"^[0-9]+>&[0-9]+$",	// TOK_REDIR_DUP_OUT
	"^[0-9]+<&-$",		// TOK_REDIR_DUP_CLOSE_IN
	"^[0-9]+>&-$",		// TOK_REDIR_DUP_CLOSE_OUT
};

/**
 * @brief Transform an input string into a series of tokens.
 *
 * @param[in] input					The raw user input to tokenize.
 * @param[out] numProcessedTokens	The number of processed tokens before and
 * after tokenization.
 * @param[out] outTokens			The \ref Token_t array to populate.
 * @param[in] lastExit				The status from the last call to tokenize.
 *
 * @return Status code of tokenization. 0 if success, 1 if general error, and
 * -1 on unclosed single quote, -2 on unclosed double quote, -3 on unclosed
 * backslash, -4 on unclosed pipe, -5 on unclosed AND, -6 on unclosed OR.
*/
int tokenizeInput(char* input, int* numProcessedTokens, Token_t** outTokens,
	int lastExit) {

	// Ensure input non-null
	if (input == NULL || *numProcessedTokens < 0) {
		return 0;
	}

	// Reallocate enough space for all tokens with buffer
	Token_t* tmp = realloc(*outTokens, TOKEN_BUFFER_SIZE * sizeof(Token_t));
	if (tmp == NULL) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	*outTokens = tmp;

	// Ensure first token is setup if this is first call
	if (lastExit == 0) {
		Token_t* firstToken = *outTokens;
		firstToken->charCount = 0;
		firstToken->type = TOK_WORD;

		// Resize buffer fail
		if (resizeBuffer(firstToken, TOKEN_CHARACTER_BUFFER_SIZE) == -1) {
			
			++(*numProcessedTokens);
			tmp = realloc(*outTokens, *numProcessedTokens * sizeof(Token_t));
			if (tmp == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}

			// Set out parameter and return
			*outTokens = tmp;
			perror("resizeBuffer");
			return 1;
		}
	}

	// Start quote status based on prev call
	QuotedStatus_t qStatus = (lastExit == -2 || lastExit == -1) \
		? (lastExit * -1) : 0;

	// Store status of running backslashes
	int backslashStatus = 0;

	// Loop over input
	char* curChar = input;
	while (*curChar) {

		// Get token currently being build
		Token_t* curToken = *outTokens + *numProcessedTokens;

		// Construct a object for the current character
		CharIn_t* curCharObj = createCharIn(*curChar, qStatus);
		if (curCharObj == NULL) {

			// Failure to construct char
			++(*numProcessedTokens);
			tmp = realloc(*outTokens, *numProcessedTokens * sizeof(Token_t));
			if (tmp == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}

			// Set out parameter and return
			*outTokens = tmp;
			perror("createCharIn");
			return 1;
		}

		// Check for double quotes
		if (*curChar == '"' && !backslashStatus && qStatus != QUOTED_SINGLE) {

			// Closing double quote and continue to next character
			if (qStatus == QUOTED_DOUBLE) {
				qStatus = QUOTED_NONE;

			// Opening double quote and continue to next character
			} else if (qStatus == QUOTED_NONE) {
				qStatus = QUOTED_DOUBLE;
			}

		// Check for single quotes
		} else if (*curChar == '\'' && !backslashStatus
			&& qStatus != QUOTED_DOUBLE) {

			// Closing double quote and continue to next character
			if (qStatus == QUOTED_SINGLE) {
				qStatus = QUOTED_NONE;

			// Opening double quote and continue to next character
			} else if (qStatus == QUOTED_NONE) {
				qStatus = QUOTED_SINGLE;
			}

		// Check for seperating tokens
		} else if (*curChar == '\n' || *curChar == '\r') {
			;

		} else if ((*curChar == ' ' || *curChar == '\t')
			&& (qStatus == QUOTED_NONE)) {

			// Check for empty tokens
			if (curToken->charCount == 0) {
				free(curCharObj);
				curChar += 1;
				backslashStatus = 0;
				continue;
			}

			// Resize current token array to exact size
			if (resizeBuffer(curToken, curToken->charCount) == -1) {

				// Failure to resize
				++(*numProcessedTokens);
				tmp = realloc(*outTokens, \
					*numProcessedTokens * sizeof(Token_t));
				if (tmp == NULL) {
					perror("realloc");
					exit(EXIT_FAILURE);
				}

				// Set out parameter and return
				*outTokens = tmp;
				perror("resizeBuffer");
				return 1;
			}

			// Set the type of the processed token
			setType(curToken);

			// Number of tokens processed has increased with closed token
			++(*numProcessedTokens);

			// Setup next token
			Token_t* nextToken = *outTokens + *numProcessedTokens;
			nextToken->charCount = 0;
			nextToken->type = TOK_WORD;
			if (resizeBuffer(nextToken, TOKEN_CHARACTER_BUFFER_SIZE) == -1) {

				// Failure to resize
				++(*numProcessedTokens);
				tmp = realloc(*outTokens, \
					*numProcessedTokens * sizeof(Token_t));
				if (tmp == NULL) {
					perror("realloc");
					exit(EXIT_FAILURE);
				}

				// Set out parameter and return
				*outTokens = tmp;
				perror("resizeBuffer");
				return 1;
			}


		// Add current character to end of the token as normal
		} else {

			// Ensure size for the extra character
			if (curToken->charCount == curToken->buffSize) {

				// Reallocate buffer with incremental size
				if (resizeBuffer(curToken, \
					curToken->buffSize + TOKEN_CHARACTER_BUFFER_SIZE) == -1) {

					// Failure to resize
					++(*numProcessedTokens);
					tmp = realloc(*outTokens, \
						*numProcessedTokens * sizeof(Token_t));
					if (tmp == NULL) {
						perror("realloc");
						exit(EXIT_FAILURE);
					}

					// Set out parameter and return
					*outTokens = tmp;
					perror("resizeBuffer");
					return 1;
				}
			}

			// Insert the character in
			curCharObj->quoted = qStatus;
			*(curToken->chars + curToken->charCount) = *curCharObj;
			curToken->charCount += 1;
		}

		// Set backslash status
		if (*curChar == '\\' && qStatus != QUOTED_SINGLE) {
			backslashStatus ^= 1;
		} else {
			backslashStatus = 0;
		}

		// Step to next character in array
		free(curCharObj);
		curChar += 1;
	}

	// Finalize last token (in event user input doens't end on whitespace)
	Token_t* lastToken = *outTokens + *numProcessedTokens;
	if (lastToken->charCount > 0 && qStatus == QUOTED_NONE) {

		// Resize current token array to exact size
		if (resizeBuffer(lastToken, lastToken->charCount) == -1) {
			
			// Failure to resize
			++(*numProcessedTokens);
			tmp = realloc(*outTokens, \
				*numProcessedTokens * sizeof(Token_t));
			if (tmp == NULL) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}

			// Set out parameter and return
			*outTokens = tmp;
			perror("resizeBuffer");
			return 1;
		}

		// Set the type of the processed token
		setType(lastToken);

		// Number of tokens processed has increased with closed token
		++(*numProcessedTokens);
	}


	// if the last token is finalized and is a pipe / separator then need to throw an error

	// if the quote isn't none then the last token is complete, num processed tokens shoudl have been eset accordingly

	// Return status code

	// Trailing quote
	if (qStatus != QUOTED_NONE) {
		return -1 * qStatus;

	// Backslash
	} else if (backslashStatus) {
		return -3;

	// Pipe
	} else if ((*outTokens + *numProcessedTokens - 1)->type == TOK_PIPE) {
		return -4;

	// AND
	} else if ((*outTokens + *numProcessedTokens - 1)->type == TOK_AND) {
		return -5;

	// OR
	} else if ((*outTokens + *numProcessedTokens - 1)->type == TOK_OR) {
		return -6;
	}

	// Input is finished, reallocate
	tmp = realloc(*outTokens, *numProcessedTokens * sizeof(Token_t));
	if (tmp == NULL) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	*outTokens = tmp;

	return 0;
}

/**
 * @brief Set the type of a \ref Token_t object baesd on its characters.
 *
 * @param[in] token The \ref Token_t to set.
*/
void setType(Token_t* token) {

	// Ensure non-null
	if (token == NULL) { errno = EINVAL; return; }

	// Copy raw characters from token into an array
	char* rawChars = extractChars(token, 0);
	if (rawChars == NULL) {
		perror("extractChars");
		return;
	}

	// Compile and check all redirect regular expressions
	regex_t regex;
	for (int i = TOK_REDIR_IN; i < TOK_AND; ++i) {

		// Compile the current regex
		if(regcomp(&regex, redirectRegexs[i], REG_EXTENDED) != 0) {
			perror("regcomp");
			exit(EXIT_FAILURE);
		}

		// Do the comparison
		if (!regexec(&regex, rawChars, 0, NULL, 0)) {
			token->type = i;
			free(rawChars);
			return;
		}
	}

	// Check pipes and separators
	if (strcmp(rawChars, "|") == 0) {
		token->type = TOK_PIPE;

	} else if (strcmp(rawChars, "&&") == 0) {
		token->type = TOK_AND;

	} else if (strcmp(rawChars, "||") == 0) {
		token->type = TOK_OR;

	} else if (strcmp(rawChars, ";") == 0) {
		token->type = TOK_SEMI;


	// Must be normal word
	} else {
		token->type = TOK_WORD;
	}

	// Free raw chars
	free(rawChars);
}

/**
 * @brief Resize the buffer for storing characters in a \ref Token_t object.
 *
 * @param[in] token 		The \ref Token_t to query.
 * @param[in] newBuffSize	The new buffer size to set.
 *
 * @return New buffer size of token object @token or -1 on failure.
*/
int resizeBuffer(Token_t* token, int newBuffSize) {

	// Ensure non-null and buffer provides space for all characters
	if (token == NULL || newBuffSize < token->charCount) { errno = EINVAL; return -1; }

	// Attempt to resize
	CharIn_t* tmp = realloc(token->chars, newBuffSize * sizeof(CharIn_t));
	if (tmp == NULL) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}

	// Set new buffer and return
	token->chars = tmp;
	token->buffSize = newBuffSize;
	return newBuffSize;
}

/**
 * @brief Returns if a \ref Token_t contains a particular character.
 *
 * \ref TokenType_t which is a word contains raw user input / words. Tests
 * if a particular token, which must be a word, contains a character.
 *
 * @param[in] token The \ref Token_t to query.
 * @param[in] c		The character to test if @token contains.
 *
 * @return 1 if character is contained, 0 if not.
*/
int containsChar(Token_t* token, char c) {

	// Ensure non-null
	if (token == NULL || token->type != TOK_WORD) { errno = EINVAL; return 0; }

	// Loop over list of characters in token
	int i = 0;
	while (i < token->charCount) {

		// Check if found search character
		if (token->chars[i].c == c) { return 1; }

		// Step
		i += 1;
	}

	// Character not found, return false
	return 0;
}

/**
 * @brief Returns if a \ref Token_t contains a particular substring.
 *
 * \ref TokenType_t which is a word contains raw user input / words. Tests
 * if a particular token, which must be a word, contains a substring.
 *
 * @param[in] token The \ref Token_t to query.
 * @param[in] sub	The substring to test if @token contains.
 *
 * @return 1 if substring is valid, 0 if not.
*/
int containsSubstring(Token_t* token, char* sub) {

	// Ensure non-null and valid word
	if (token == NULL || sub == NULL || token->type != TOK_WORD) {
		errno = EINVAL;
		return 0;
	}

	// Copy raw characters from token into an array
	char* rawChars = malloc(sizeof(char) * token->charCount + 1);
	if (rawChars == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// Loop over and copy in
	int i = 0;
	while (i < token->charCount) {

		// Copy in character
		rawChars[i] = token->chars[i].c;

		// Step
		i += 1;
	}

	// Null terminate array copy for raw data
	rawChars[i] = '\0';

	// Test if characters contains substring
	int tmp = !(strstr(rawChars, sub) == NULL);

	// Free temp copy of characters
	free(rawChars);

	// Return if found
	return tmp;
}

/**
 * @brief Extracts the raw characters from \ref CharIn_t objects within a
 * \ref Token_t object.
 *
 * Dynamically allocates and returns a list of characters, extracted from all
 * \ref CharIn_t objects within a \ref Token_t object.
 *
 * @param[in] token The \ref Token_t to extract text from.
 *
 * @return NULL if token is invalid, else the deep copy of raw chars.
*/
char* extractChars(Token_t* token, int expand) {

	// Ensure non-null
	if (token == NULL) { errno = EINVAL; return NULL; }

	// Copy raw characters from token into an array
	char* rawChars = malloc(sizeof(char) * token->charCount + 1);
	if (rawChars == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// Loop over and copy in
	int i = 0;
	while (i < token->charCount) {

		// Copy in character
		rawChars[i] = token->chars[i].c;

		// Step
		++i;
	}

	// Null terminate array copy for raw data
	rawChars[i] = '\0';

	// Return the list
	return rawChars;
}

/**
 * @brief Returns if a \ref Token_t is a redirect operator or not.
 *
 * @param[in] token The \ref Token_t to query.
 *
 * @return 1 if redirect operator, 0 if not, 0 if error.
 *
 * @note Returns 0 if @token is null.
*/
int isRedirect(Token_t* token) {

	// Ensure non-null
	if (token == NULL) { return 0; }

	return token->type > TOK_PIPE && token->type < TOK_AND;
}

/**
 * @brief Returns if a \ref Token_t is a separator operator or not.
 *
 * @param[in] token The \ref Token_t to query.
 *
 * @return 1 if separator operator, 0 if not, 0 if error.
 *
 * @note Returns 0 if @token is null.
*/
int isSeparator(Token_t* token) {

	// Ensure non-null
	if (token == NULL) { return 0; }

	return token->type == TOK_AND || token->type == TOK_OR;
}

/**
 * @brief For debug, just print the contents of a token.
 *
 * @param[in] token The \ref Token_t to query.
*/
void printToken(Token_t* token) {

	// Ensure non-null
	if (token == NULL) { errno = EINVAL; return; }

	printf("Token type: %d\n", token->type);
	printf("Token length: %d\n", token->charCount);
	printf("Buffer length: %d\n", token->buffSize);

	printf("Token content: \n");

	// Loop over list of characters in token
	int i = 0;
	while (i < token->charCount) {

		printf("%c %d \n", token->chars[i].c, token->chars[i].quoted);

		// Step
		i += 1;
	}
	printf("\n");
}

/*
 * @brief Frees all memory from a list of tokens.
 *
 * Frees all memory from all tokens in a list, including each character object
 * from the list within each token.
 * 
 * @param[in] tokens	The list of tokens to free.
 * @param[in] numTokens	The number of tokens in the list to free.
*/
void freeTokens(Token_t* tokens, int numTokens) {

	for (int i = 0; i < numTokens; ++i) {

		// Free all characters within this token
		free(tokens[i].chars);
	}
	free(tokens);
}
