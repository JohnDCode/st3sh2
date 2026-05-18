#ifndef CHAR_IN_H
#define CHAR_IN_H

/**
 * @brief Stores if a character is within a quoted region.
 * 
 * Stores if a character is part of a double or single quoted region, or if it
 * is escaped by a backslash. Character is parsed into commands differently
 * based on such status.
 */
typedef enum QuotedStatus_t {
	//QUOTED_BACK_SLASH,	/**< Character is escaped with a backslash. */
	QUOTED_NONE,			/** <Character is not within a quoted region. */
	QUOTED_SINGLE,			/**< Character is within a double quoted region. */
	QUOTED_DOUBLE			/**< Character is within a single quoted region. */
} QuotedStatus_t;

/**
 * @brief Stores a single character from user input.
 *
 * Stores the actually inputted character, as well as the status of the
 * character relative to other quoted blocks using \ref QuotedStatus_t.
*/
typedef struct CharIn_t {
	char c;					/**< The inputted character. */
	QuotedStatus_t quoted;	/**< Status of quote block containing character. */
} CharIn_t;

CharIn_t* createCharIn(char c, QuotedStatus_t quotedStatus);

#endif
