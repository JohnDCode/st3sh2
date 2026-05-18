#ifndef PIPELINE_H
#define PIPELINE_H

#include "parse/cmd/cmd.h"

/**
 * @brief Stores command data for a group of cmds that are piped together.
*/
typedef struct Pipeline_t {
	Cmd_t* cmds;	/**< The list of commands that are piped together. */
	int cmdCount;	/**< The number of commands in this pipeline. . */
} Pipeline_t;

#endif
