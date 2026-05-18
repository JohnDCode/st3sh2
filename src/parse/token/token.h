#ifndef TOKEN_H
#define TOKEN_H

#include "parse/charIn/charIn.h"

/**
 * @brief Stores the type of a command line token.
 * 
 * Stores if a character is a redirection symbol, pipe symbol, seperator,
 * actual argument (word), etc.
 */
typedef enum TokenType_t {
	TOK_WORD,					/**< Basic argument (word). */

	TOK_PIPE,					/**< '|' pipe symbol. */

	TOK_REDIR_IN,				/**< '<' redirect. */
	TOK_REDIR_OUT,				/**< '>' redirect. */
	TOK_REDIR_APPEND,			/**< '>>' redirect. */
	TOK_REDIR_HERE,				/**< '<<' redirect. */
	TOK_REDIR_RW,				/**< '<>' redirect. */

	TOK_REDIR_DUP_IN,			/**< '<&' redirect. */
	TOK_REDIR_DUP_OUT,			/**< '>&' redirect. */
	TOK_REDIR_DUP_CLOSE_IN,		/**< '<&-' redirect. */
	TOK_REDIR_DUP_CLOSE_OUT,	/**< '>&-' redirect. */

	TOK_AND,					/**< '&&' seperator. */
	TOK_OR,						/**< '||' seperator. */
	TOK_SEMI,					/**< ';' seperator. */
	TOK_DUMMY
} TokenType_t;

/**
 * @brief Stores a single token from user input.
 *
 * Stores the actually inputted characters as an array of type \ref CharIn_t
 * as well as the type of the token using \ref TokenType_t.
*/
typedef struct Token_t {
	CharIn_t* chars;	/**< The list of characters constituting the token. */
	int charCount;		/**< The number of characters in the token. */
	int buffSize;		/**< Current size of buffer for chars array. */
	TokenType_t type;	/**< The type of token. */
} Token_t;

int tokenizeInput(char* input, int* numProcessedTokens, Token_t** outTokens,
	int lastExit);
void setType(Token_t* token);
int resizeBuffer(Token_t* token, int newBuffSize);
int containsChar(Token_t* token, char c);
int containsSubstring(Token_t* token, char* sub);
char* extractChars(Token_t* token, int expand);
int isRedirect(Token_t* token);
int isSeparator(Token_t* token);
void printToken(Token_t* token);
void freeTokens(Token_t* tokens, int numTokens);

#endif
