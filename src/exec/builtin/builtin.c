#include "exec/builtin/builtin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "parse/cmd/cmd.h"
#include "about/art.h"
#include "about/about.h"

/** Number of builtin cmds in st3sh. */
#define numBuiltinCmds 5

/** All cmds builtin to st3sh. */
static const builtinCmd builtinCommands[numBuiltinCmds] = {
    {"about", "Prints details on st3sh", aboutCmd},
    {"cd", "Changes the cwd of the shell process", cdCmd},
    {"exit", "Exits the shell", exitCmd},
    {"help", "Prints information on each builtin shell command", helpCmd},
    {"version", "Prints the current st3sh instance version", versionCmd}
};

/*
 * @brief Returns if a cmd is bulitin or not to st3sh.
 *
 * @param[in] cmd The cmd to query its state as builtin or not.
 *
 * @return Nonzero if the cmd is builtin to st3sh, 0 if not.
*/
int isBuiltin(Cmd_t* cmd) {

	// Ensure cmd is valid
	if (cmd == NULL || cmd->argc == 0 || cmd->argv == NULL \
		|| cmd->argv[0] == NULL) {
		errno = EINVAL; 
		return 0;
	}

	// Get first arg
	char* proc = cmd->argv[0];

	// Loop over all builtin commands
	for (size_t i = 0; i < numBuiltinCmds; ++i) {

		// Check if found with this proc
		if (strcmp(proc, builtinCommands[i].name) == 0) {
			return 1;
		}
	}

	// Not bulitin
	return 0;
}

/*
 * @brief Executes a builtin cmd.
 *
 * @param[in] cmd The cmd to execute as builtin.
 *
 * @return Exit code from the builtin command, 1 if invalid builtin command.
*/
int execBuiltin(Cmd_t* cmd) {

	// Ensure cmd is valid
	if (cmd == NULL || cmd->argc == 0 || cmd->argv == NULL \
		|| cmd->argv[0] == NULL) {
		errno = EINVAL;
		return EXIT_FAILURE;
	}

	// Cache old fds
	int cachedStdin = dup(STDIN_FILENO);
	int cachedStdout = dup(STDOUT_FILENO);

	// Apply redirects for this cmd
	applyRedirects(cmd);

	// Get first arg
	char* proc = cmd->argv[0];

	// Find and run bulitin cmd
	for (int i = 0; i < numBuiltinCmds; ++i) {
		if (strcmp(proc, builtinCommands[i].name) == 0) {
			
			// Run and return
			int status = builtinCommands[i].callback(cmd);

			// Restore fds
			dup2(cachedStdin, STDIN_FILENO);
			dup2(cachedStdout, STDOUT_FILENO);
			close(cachedStdin);
			close(cachedStdout);

			if (status != 0) { perror(proc); }

			// Return status from cmd
			return status;
		}
	}

	// Not builtin / not found in list
	errno = EINVAL; 
	return EXIT_FAILURE;
}

/*
 * @brief Builtin cmd 'about'.
 *
 * Prints general info on st3sh.
*/
int aboutCmd(Cmd_t* cmd) {
	
	// Just print help details
	printf("\n");
	printLogo();
	printf("\n\n");
	printf("%31s%s\n", "", "st3sh: A POSIX x84_64 shell");
	printf("%*sCreated By %s\n", (int)(90 - strlen(AUTHOR) - 11) / 2, "", AUTHOR);
	printf("%*sVersion # %s\n", (int)(90 - strlen(VERSION) - 10) / 2, "", VERSION);
	printf("%13s%s\n\n\n","", "github.com/johndcode | linkedin.com/in/johndcode | johndcode.com");
	return 0;
}

/*
 * @brief Builtin cmd 'cd'.
 *
 * Changes cwd of st3sh.
*/
int cdCmd(Cmd_t* cmd) {

	// Check args
	if (cmd == NULL || cmd->argv == NULL || cmd->argc < 2) {
		errno = EINVAL;
		return 1;
	}

	// Chang cwd and return
	return chdir(cmd->argv[1]);
}

/*
 * @brief Builtin cmd 'exit'.
 *
 * Exits st3sh.
*/
int exitCmd(Cmd_t* cmd) {
	exit(EXIT_SUCCESS);
	return 0;
}

/*
 * @brief Builtin cmd 'help'.
 *
 * Prints help info on st3sh.
*/
int helpCmd(Cmd_t* cmd) {
	
	// Print help menu / descriptions of each builtin shell command
	printf("st3sh commands: \n\n");
	for (int i = 0; i < numBuiltinCmds; ++i) {
		printf("%s" ": " "%s\n", builtinCommands[i].name, builtinCommands[i].description);
	}
	printf("\n");
	printf("Run applications with: filename [args]\n");
	return 0;
}

/*
 * @brief Builtin cmd 'version'.
 *
 * Prints version info on st3sh.
*/
int versionCmd(Cmd_t* cmd) {
	printf("%s\n", VERSION);
	return 0;
}
