#include "parse/cmd/cmd.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "parse/token/token.h"

/*
 * @brief Applies all redirects from a cmd to the process.
 *
 * @param[in] cmd The cmd whose redirects are to be applied.
 *
 * @note All redirects should be well formed. I.e all in redirects should have
 * non-null and null-terminated strings.
*/
void applyRedirects(Cmd_t* cmd) {

	// Ensure non-null
	if (cmd == NULL) {
		errno = EINVAL;
		return;
	}

	// Loop over all redirects in this command
	for (int i = 0; i < cmd->redirCount; i++) {
		Redirect_t *curRedir = cmd->redirs + i;

		// Get the left hand side file descriptor from the redirect if exists
		int leftFd = STDOUT_FILENO;

		// Has an explicit left fd
		if (curRedir->fd != -1) {
			leftFd = curRedir->fd;

		// Use stdin
		} else if (curRedir->type == TOK_REDIR_IN \
			|| curRedir->type == TOK_REDIR_HERE || \
			curRedir->type == TOK_REDIR_RW) {
			leftFd = STDIN_FILENO;
		}


		// Apply redirect based on type
		int newFd;
		switch (curRedir->type) {
			
			// <, >, >>, <>
			case TOK_REDIR_IN:
			case TOK_REDIR_OUT:
			case TOK_REDIR_APPEND:
			case TOK_REDIR_RW:

			// Open new fd
			if (curRedir->type == TOK_REDIR_IN) {
				newFd = open(curRedir->target, O_RDONLY);
			} else if (curRedir->type == TOK_REDIR_OUT) {
				newFd = open(curRedir->target, \
					O_WRONLY | O_CREAT | O_TRUNC, 0644);
			} else if (curRedir->type == TOK_REDIR_APPEND) {
				newFd = open(curRedir->target, \
					O_WRONLY | O_CREAT | O_APPEND, 0644);
			} else {
				newFd = open(curRedir->target, O_RDWR | O_CREAT, 0644);
			}

			if (newFd == -1) {
				perror(curRedir->target);
				exit(EXIT_FAILURE);
			}

			if (dup2(newFd, leftFd) == -1) {
				perror("dup2");
				exit(EXIT_FAILURE);
			}

			if (close(newFd) == -1) {
				perror("close");
				exit(EXIT_FAILURE);
			}
			break;


			// <<
			case TOK_REDIR_HERE: {

				// Read target as stdin
				int p[2];
				if (pipe(p) == -1) { perror("pipe"); exit(EXIT_FAILURE); }
				
				if (write(p[1], curRedir->target,
					strlen(curRedir->target)) == -1) {
					perror("write");
					exit(EXIT_FAILURE);
				}

				if (close(p[1]) == -1) { perror("close"); exit(EXIT_FAILURE); }

				if (dup2(p[0], leftFd) == -1) {
					perror("dup2");
					exit(EXIT_FAILURE);
				}

				if (close(p[0]) == -1) { perror("close"); exit(EXIT_FAILURE); }

				break;
			}


			// n<&m, n>&m
			case TOK_REDIR_DUP_IN:
			case TOK_REDIR_DUP_OUT:

			// Duplicate rightFd (targetFd)
			if (dup2(curRedir->targetFd, leftFd) == -1) {
				perror("dup2");
				exit(EXIT_FAILURE);
			}
			break;


			// n<&-, n>&-
			case TOK_REDIR_DUP_CLOSE_IN:
			case TOK_REDIR_DUP_CLOSE_OUT:

			if (close(leftFd) == -1) {
				perror("close");
				exit(EXIT_FAILURE);
			}
			break;
		}
	}
}
