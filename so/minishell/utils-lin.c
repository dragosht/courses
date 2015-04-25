/**
 * Operating Sytems 2013 - Assignment 2
 *
 */

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

#define READ		0
#define WRITE		1


/**
 * Concatenate parts of the word to obtain the command
 */
static char *get_word(word_t *s)
{
    int string_length = 0;
    int substring_length = 0;

    char *string = NULL;
    char *substring = NULL;

    while (s != NULL) {
        substring = strdup(s->string);

        if (substring == NULL) {
            return NULL;
        }

        if (s->expand == true) {
            char *aux = substring;
            substring = getenv(substring);

            /* prevents strlen from failing */
            if (substring == NULL) {
                substring = calloc(1, sizeof(char));
                if (substring == NULL) {
                    free(aux);
                    return NULL;
                }
            }

            free(aux);
        }

        substring_length = strlen(substring);

        string = realloc(string, string_length + substring_length + 1);
        if (string == NULL) {
            if (substring != NULL)
                free(substring);
            return NULL;
        }

        memset(string + string_length, 0, substring_length + 1);

        strcat(string, substring);
        string_length += substring_length;

        if (s->expand == false) {
            free(substring);
        }

        s = s->next_part;
    }

    return string;
}

/**
 * Concatenate command arguments in a NULL terminated list in order to pass
 * them directly to execv.
 */
static char **get_argv(simple_command_t *command, int *size)
{
    char **argv;
    word_t *param;

    int argc = 0;
    argv = calloc(argc + 1, sizeof(char *));
    assert(argv != NULL);

    argv[argc] = get_word(command->verb);
    assert(argv[argc] != NULL);

    argc++;

    param = command->params;
    while (param != NULL) {
        argv = realloc(argv, (argc + 1) * sizeof(char *));
        assert(argv != NULL);

        argv[argc] = get_word(param);
        assert(argv[argc] != NULL);

        param = param->next_word;
        argc++;
    }

    argv = realloc(argv, (argc + 1) * sizeof(char *));
    assert(argv != NULL);

    argv[argc] = NULL;
    *size = argc;

    return argv;
}

/**
 * Internal change-directory command.
 */
static bool shell_cd(simple_command_t *s, int level, command_t *father)
{
    int ret = 0;
    char* path = get_word(s->params);
	ret = chdir(path);

	if (s->out != NULL) {
	    char* fout = get_word(s->out);
	    int fd = open(fout, O_CREAT, 0644);
	    close(fd);
	    free(fout);
	}
	if (s->err != NULL) {
	    char* ferr = get_word(s->err);
	    int fd = open(ferr, O_CREAT, 0644);
	    close(fd);
	    free(ferr);
	}

	free(path);
	return ret;
}

/**
 * Internal environment variable definition routine
 */
static bool do_env_variable_params(simple_command_t *s, int level, command_t *father)
{
    int status = 0;
    int size = 0;
    char** args = get_argv(s, &size);

    assert(size >= 2);
    assert(!strcmp(args[1], "="));

    if (size >= 3) {
        setenv(args[0], args[2], 1);
    } else {
        setenv(args[0], "", 1);
    }

    return status;
}

static bool do_env_variable_verb(simple_command_t *s, int level, command_t *father)
{
    int status = 0;
    char* cmd = get_word(s->verb);
    char* eqmark = strchr(cmd, '=');
    assert(eqmark != NULL);

    /* Don't do this at home ... */
    *eqmark = 0;
    eqmark++;
    setenv(cmd, eqmark, 1);
    *eqmark = '=';

    free(cmd);
    return status;
}


/**
 * Handle the file redirections.
 */
void do_io_redirections(simple_command_t *s, command_t* father)
{
    command_t* gfather = NULL;
    char* fin = NULL;
    char* fout = NULL;
    char* ferr = NULL;
    int flags = 0;
    int fd = 0;

    assert(s != NULL);

    if (father != NULL && father->op == OP_PIPE) {
        assert(father->aux != NULL);
        pipefd_t* pfd = father->aux;

        if (s->up == father->cmd1) {
            //Write on this pipe
            dup2(pfd->fdwrite, STDOUT_FILENO);
            close(pfd->fdwrite);
            close(pfd->fdread);

        } else if (s->up == father->cmd2) {
            //Read from this pipe
            dup2(pfd->fdread, STDIN_FILENO);
            close(pfd->fdread);
            close(pfd->fdwrite);

            //Handle multiple pipes ...

            //Boy ... That's been tough!
            gfather = father->up;
            if (gfather != NULL && gfather->op == OP_PIPE) {
                pipefd_t* gpfd = NULL;
                assert(gfather->aux != NULL);
                gpfd = gfather->aux;
                dup2(gpfd->fdwrite, STDOUT_FILENO);
                close(gpfd->fdwrite);
                close(gpfd->fdread);
            }
        }
    }

    //Hopefully I don't get into something like: cat file | less < log (!?)

    if (s->in != NULL) {
        fin = get_word(s->in);
        fd = open(fin, O_RDONLY, 0644);
        dup2(fd, STDIN_FILENO);
        close(fd);
        free(fin);
    }

    if (s->out != NULL) {
        fout = get_word(s->out);
    }
    if (s->err != NULL) {
        ferr = get_word(s->err);
    }

    if (fout != NULL && ferr != NULL && !strcmp(fout, ferr)) {
        fd = open(fout, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
        free(fout);
        free(ferr);
    } else {
        if (fout != NULL) {
            flags = O_WRONLY | O_CREAT;
            if (s->io_flags & IO_OUT_APPEND) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }

            fd = open(fout, flags, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            free(fout);
        }
        if (ferr != NULL) {
            flags = O_WRONLY | O_CREAT;
            if (s->io_flags & IO_ERR_APPEND) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }

            fd = open(ferr, flags, 0644);
            dup2(fd, STDERR_FILENO);
            close(fd);
            free(ferr);
        }
    }
}

int do_wait(simple_command_t *s, int pid, command_t *father)
{
    int status;

    assert(s != NULL);

    if (father == NULL) {
        waitpid(pid, &status, 0);
    } else {
        switch(father->op) {
        case OP_SEQUENTIAL:
        case OP_CONDITIONAL_NZERO:
        case OP_CONDITIONAL_ZERO:
            waitpid(pid, &status, 0);
            break;
        default:
            break;
        }
    }

    return status;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
    int size = 0;
    char** args = NULL;
    char* cmd = 0;

    assert(s != NULL);

    cmd = get_word(s->verb);

    if (!strcmp(cmd, "exit") ||
        !strcmp(cmd, "quit")) {
        free(cmd);
        return SHELL_EXIT;
    }

    if (!strcmp(cmd, "cd")) {
        free(cmd);
        return shell_cd(s, level, father);
    }

    if (strchr(cmd, '=') != NULL) {
        free(cmd);
        return do_env_variable_verb(s, level, father);
    }

    if (s->params != NULL) {
        char* first = get_word(s->params);
        if (!strcmp(first, "=")) {
            free(cmd);
            free(first);
            return do_env_variable_params(s, level, father);
        }
    }

    args = get_argv(s, &size);

    pid_t pid = fork();
    if (pid > 0) {
        free(cmd);
        free(args);
        return do_wait(s, pid, father);
    } else if (pid == 0) {
        do_io_redirections(s, father);
        if (execvp(cmd, args) == -1) {
            fprintf(stderr, "Execution failed for '%s'\n", cmd);
            return -1;
        }
    } else {
        perror("fork()");
        exit(EXIT_FAILURE);
    }

	return 0;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_sequentially(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
    int status2 = 0;
    parse_command(cmd1, level, father);
    status2 = parse_command(cmd2, level, father);
    return status2;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
    command_t* gfather = NULL;
    int crtstatus;
    int status;
    int i;

    parse_command(cmd1, level, father);
    parse_command(cmd2, level, father);

    gfather = father->up;
    if (gfather == NULL || gfather->op != OP_PARALLEL) {
        //wait for all the processes running in parrallel starting from here ...
        int processes = 2;
        command_t* crt = father;
        while (crt->cmd1 != NULL && crt->cmd1->op == OP_PARALLEL) {
            crt = crt->cmd1;
            processes++;

        }

        /* These are not very nice here */
        for (i = 0; i < processes; i++) {
            wait(&crtstatus);
            status = status || crtstatus;
        }
    }

	return status;
}

/**
 * Process two commands conditionally
 */
static bool do_conditional_nzero(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{

    int status1 = 0;
    int status2 = 0;
    status1 = parse_command(cmd1, level, father);
    if (status1) {
        status2 = parse_command(cmd2, level, father);
    } else {
        status2 = status1;
    }
    return status2;
}

static bool do_conditional_zero(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
    int status1 = 0;
    int status2 = 0;
    status1 = parse_command(cmd1, level, father);
    if (!status1) {
        status2 = parse_command(cmd2, level, father);
    } else {
        status2 = status1;
    }

    return status2;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
    int i;
    int status = 0;
    int crtstatus = 0;
    int pp[2];
    command_t* gfather = NULL;
    pipefd_t* pfd = malloc(sizeof(pipefd_t));

    pipe(pp);

    pfd->fdwrite = pp[1];
    pfd->fdread = pp[0];
    father->aux = pfd;

    parse_command(cmd1, level, father);
    parse_command(cmd2, level, father);

    //The shell does not need these ...
    close(pfd->fdwrite);
    close(pfd->fdread);

    gfather = father->up;
    if (gfather == NULL || gfather->op != OP_PIPE ) {
        //wait for all the pipy processes
        int processes = 2;
        command_t* crt = father;
        while (crt->cmd1 != NULL) {
            /*
             * Don't even bother checking for other operations.
             * The | operator has the highest priority according to the holy parser.
             * Apparently if I wait for every group of 2 pipe-connected processes
             * I get a huge delay on test15.
             */
            crt = crt->cmd1;
            processes++;

        }

        for (i = 0; i < processes; i++) {
            wait(&crtstatus);
            status = status || crtstatus;
        }
    }

    //This should be a good place to wipe this out ...
    free(father->aux);
    father->aux = NULL;

	return status;
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
    assert(c != NULL);

	if (c->op == OP_NONE) {
	    return parse_simple(c->scmd, level, father);
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
	    return do_sequentially(c->cmd1, c->cmd2, level + 1, c);
		break;

	case OP_PARALLEL:
		return do_in_parallel(c->cmd1, c->cmd2, level + 1, c);
		break;

	case OP_CONDITIONAL_NZERO:
		return do_conditional_nzero(c->cmd1, c->cmd2, level + 1, c);
		break;

	case OP_CONDITIONAL_ZERO:
		return do_conditional_zero(c->cmd1, c->cmd2, level + 1, c);
		break;

	case OP_PIPE:
	    return do_on_pipe(c->cmd1, c->cmd2, level + 1, c);
		break;

	default:
		assert(false);
	}

	return 0;
}

/**
 * Readline from mini-shell.
 */
char *read_line()
{
	char *instr;
	char *chunk;
	char *ret;

	int instr_length;
	int chunk_length;

	int endline = 0;

	instr = NULL;
	instr_length = 0;

	chunk = calloc(CHUNK_SIZE, sizeof(char));
	if (chunk == NULL) {
		fprintf(stderr, ERR_ALLOCATION);
		return instr;
	}

	while (!endline) {
		ret = fgets(chunk, CHUNK_SIZE, stdin);
		if (ret == NULL) {
			break;
		}

		chunk_length = strlen(chunk);
		if (chunk[chunk_length - 1] == '\n') {
			chunk[chunk_length - 1] = 0;
			endline = 1;
		}

		ret = instr;
		instr = realloc(instr, instr_length + CHUNK_SIZE + 1);
		if (instr == NULL) {
			free(ret);
			return instr;
		}
		memset(instr + instr_length, 0, CHUNK_SIZE);
		strcat(instr, chunk);
		instr_length += chunk_length;
	}

	free(chunk);

	return instr;
}

