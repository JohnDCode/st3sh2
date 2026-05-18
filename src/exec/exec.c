#include "exec/exec.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "exec/builtin/builtin.h"
#include "parse/redirect/redirect.h"
#include "parse/token/token.h"
#include "parse/cmd/cmd.h"

/**
 * @brief Naivley executes a single, individual command.
 *
 * @param[in] cmd The command object to execute.
 * 
 * @return Exit status of the executed command.
*/
int execCmd(Cmd_t* cmd) {

	// Ensure cmd is valid
	if (cmd == NULL || cmd->argc == 0 || cmd->argv == NULL \
		|| cmd->argv[0] == NULL) {
		errno = EINVAL;
		return EXIT_FAILURE;
	}

	// Check if builtin command
	if (isBuiltin(cmd)) {
		return execBuiltin(cmd);
	}

	// Fork and ensure success
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	// Child process
	if (pid == 0) {

		// Apply redirects
		applyRedirects(cmd);

		// Execute
		execvp(cmd->argv[0], cmd->argv);

		// Exec failed
		perror(cmd->argv[0]);
		return 1;
	}

	// Parent process

	// Return exit status of child proc
	int childStatus;
	waitpid(pid, &childStatus, 0);
	if (WIFEXITED(childStatus)) {
		return WEXITSTATUS(childStatus);
	}
	return 1;
}

/**
 * @brief Executes an entire pipeline (group of commands).
 *
 * @param[in] pipeline The pipeline object to execute.
 * 
 * @return Exit status of the last command executed.
*/
int execPipeline(Pipeline_t* pipeline) {

	// Base cases
	if (pipeline == NULL || pipeline->cmds == NULL \
		|| pipeline->cmdCount == 0) {
		errno = EINVAL;
		return EXIT_FAILURE;
	} else if (pipeline->cmdCount == 1) {
		return execCmd(pipeline->cmds);
	}

	// Construct cmdCount - 1 pipes between each pair of commands
	int **pipes = malloc((pipeline->cmdCount - 1) * sizeof(int*));
	if (pipes == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < pipeline->cmdCount - 1; ++i) {
		pipes[i] = malloc(sizeof(int) * 2);
		if (pipes[i] == NULL) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		// Construct the pipe
		if (pipe(pipes[i]) < 0) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}

	// Consturct a list of PID's for all commands
	pid_t *pids = malloc(pipeline->cmdCount * sizeof(pid_t));
	if (pids == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// Loop over all commands
	for (int i = 0; i < pipeline->cmdCount; ++i) {

		// Setup in/out file descriptors for current pipe
			// Connect to previous pipe or default fd if at begin/end of list
		int inFd = (i == 0) ? STDIN_FILENO : pipes[i - 1][0];
		int outFd =
			(i == pipeline->cmdCount - 1) ? STDOUT_FILENO : pipes[i][1];

		// Fork
		pids[i] = fork();
		if (pids[i] < 0) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		// Child process
		if (pids[i] == 0) {

			// Set file descriptors
			(inFd != STDIN_FILENO) ? dup2(inFd, STDIN_FILENO) : 0; 
			(outFd != STDOUT_FILENO) ? dup2(outFd, STDOUT_FILENO) : 0;

			// Close all pipes here
			for (int j = 0; j < pipeline->cmdCount - 1; ++j) {
				close(pipes[j][0]);
				close(pipes[j][1]);
			}

			// Apply redirects
			applyRedirects(pipeline->cmds + i);

			// Check if builtin
			if (isBuiltin(pipeline->cmds + i)) {
				exit(execBuiltin(pipeline->cmds + i));
			}

			// Execute
			execvp(pipeline->cmds[i].argv[0], pipeline->cmds[i].argv);

			// Exec failed
			perror(pipeline->cmds[i].argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	// Close all pipes
	for (int j = 0; j < pipeline->cmdCount - 1; ++j) {
		close(pipes[j][0]);
		close(pipes[j][1]);
	}

	// Wait for last command to finish
	int childStatus;
	for (int i = 0; i < pipeline->cmdCount; ++i) {

		// Set status
		waitpid(pids[i], &childStatus, 0);
		childStatus = WIFEXITED(childStatus) ? WEXITSTATUS(childStatus) : 1;
	}
	
	// Free resources, return last returned status
	free(pids);
	free(pipes);
	return childStatus;
}

/**
 * @brief Executes a separator as a group of pipelines.
 *
 * @param[in] separator	The separator object to execute.
 * 
 * @return Exit status of the last command in the last pipeline.
*/
int execSeparator(Separator_t* separator) {

	// Base case
	if (separator == NULL || separator->pipes == NULL || \
		separator->pipeCount == 0) {
		errno = EINVAL;
		return EXIT_FAILURE;
	}

	// Execute first pipeline
	int lastExit = execPipeline(separator->pipes);

	// Loop over rest of pipelines
	for (int i = 1; i < separator->pipeCount; ++i) {

		// Get current operator
		TokenType_t curOp = separator->operators[i - 1];

		// Check if can skip this pipeline
		if (curOp == TOK_AND && lastExit != 0 ||
			curOp == TOK_OR && lastExit == 0) {
			continue;
		}

		// Execute the pipeline
		lastExit = execPipeline(separator->pipes + i);
	}

	// Return the last pipes exit status
	return lastExit;
}
