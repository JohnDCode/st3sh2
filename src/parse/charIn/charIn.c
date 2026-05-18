#include "parse/charIn/charIn.h"

#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Constructs and returns a new \ref CharIn_t object.
 *
 * @param[in] c				User inputted character.
 * @param[in] quotedStatus	Quote block character is within.
 * 
 * @return Pointer to constructed \ref CharIn_t object.
 * 
 * Dynamically allocates a new \ref CharIn_t object with a character and quote
 * block. Returns pointer to the constructed object.
*/
CharIn_t* createCharIn(char c, QuotedStatus_t quotedStatus) {

	// Attempt to allocate memory
	CharIn_t* charIn = malloc(sizeof(CharIn_t));
	if (charIn == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// Populate fields and return
	charIn->c = c;
	charIn->quoted = quotedStatus;
	return charIn;
}
