#ifndef BUILTIN_H
#define BUILTIN_H

#include "parse/cmd/cmd.h"

/** Function pointer for all builtin commands. */
typedef int (*builtinFunc)(Cmd_t*);

/**
 * @brief Struct for all builtin commands.
*/
typedef struct {
    char *name; /**< The name of the builtin cmd. */
    char *description; /**< The description of the builtin cmd. */
    builtinFunc callback; /**< Function pointer to exec the cmd.. */
} builtinCmd;

int isBuiltin(Cmd_t* cmd);
int execBuiltin(Cmd_t* cmd);

int aboutCmd(Cmd_t* cmd);
int cdCmd(Cmd_t* cmd);
int exitCmd(Cmd_t* cmd);
int helpCmd(Cmd_t* cmd);
int versionCmd(Cmd_t* cmd);

#endif
