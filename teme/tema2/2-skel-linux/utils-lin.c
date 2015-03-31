/**
 * Operating Sytems 2013 - Assignment 2
 * Nume Prenume
 * Grupa
 */

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
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

static bool shell_cd(word_t *dir)
{
	int rc=chdir(get_word(dir));
	if(rc<0){
	/* verificare conditie de eroare */
		fprintf(stderr, "Error on chdir\n");
		return rc;
	}

	return 0;
}

/**
 * Internal exit/quit command.
 */
static void shell_exit()
{
	/* execute exit/quit */

	exit(SHELL_EXIT);
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */


static int parse_simple( simple_command_t *s, int level, command_t *father)
{
	char** cmd;
	int size, status, fd1, fd2, fd3, rc;
	pid_t pid;
	word_t *std_in;
	word_t *std_out;
	word_t *std_err;


	/* verificare conditie de eroare */
	DIE( s == NULL ,"Error on parse_simple: NULL command");
	DIE( father == NULL ,"Error on parse_simple: NULL father");


	cmd=get_argv(s,&size);
	/* verific pentru comanda cd */
	if(strcmp("cd",cmd[0]) == 0){
		if(s->out != NULL){
			open(s->out->string, O_WRONLY | O_CREAT | O_TRUNC, 00700);
			return shell_cd(s->params);
		}
		else return shell_cd(s->params);
	}
	/* setez variabila de environment */
	else if(s->verb->next_part != NULL && s->verb !=NULL && strcmp( s->verb->next_part->string, "=" ) == 0){
			rc=setenv( s->verb->string, s->verb->next_part->next_part->string, 1 );
			return rc;
	}
	/* cazul de exit */
	else if( (strcmp("quit", cmd[0]) == 0) || (strcmp( "exit", cmd[0]) == 0)){
		shell_exit();
	}
	else{
		if(cmd == NULL)
			return -1;
		pid=fork();

		/* verificare conditie de eroare */
		DIE(pid<0,"Error on fork()");
		if (pid ==0) {
		/* duplicare file descriptori in functie de caz */
			std_in = s->in;
			std_out = s->out;
			std_err = s->err;
			if(std_in != NULL){
				fd1 = open( get_word( std_in ), O_RDONLY);
				dup2( fd1, 0);
				close( fd1 );
			}
			if( std_err != NULL && std_out == NULL && s->io_flags == 0){
				fd3 = creat( get_word( std_err), 0644);
				dup2( fd3, 2);
				close( fd3 );
			}else if( std_err != NULL && std_out == NULL && s->io_flags == 2){
				fd3 = open( get_word( std_err ),O_WRONLY | O_CREAT | O_APPEND, 0644);
				dup2( fd3, 2);
				close( fd3 );
			}
			if(std_out != NULL && std_err == NULL && s->io_flags == 0){
				fd2 = creat( get_word( std_out), 0644);
				dup2( fd2, 1);
				close( fd2 );
			}else if( std_out != NULL && std_err == NULL && s->io_flags == 1){
				fd2 = open( get_word( std_out ),O_WRONLY | O_CREAT | O_APPEND, 0644);
				dup2( fd2,1 );
				close( fd2 );
			}	
			if( std_err != NULL && std_out != NULL){
				fd3 = creat( get_word( std_err), 0644);
				dup2( fd3, 1);
				dup2( fd3, 2);
				close( fd3 );
			}
			rc = execvp( cmd[0], (char *const *) cmd);
			DIE(rc<0,"Error on execvp");
		}
		else {
			/* parent process */
			waitpid (pid, &status, 0);
			return WEXITSTATUS( status );
		}

	}

	return 0; 
}

static bool do_in_parallel( command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	int rc, status;
	pid_t pid;

	pid = fork();

	/* verificare conditie de eroare */
	DIE (pid == -1, "Error on fork()");
	if (pid == 0) {
		/* comanda1 este executata de copil*/
		rc = parse_command( cmd1, level+1, father);
		if(rc < 0)
			exit(EXIT_FAILURE);
	}
	else {
		/* comanda 2 este executata de parinte */
		rc = parse_command( cmd2, level+1, father);
		if(rc < 0 )
			exit( EXIT_FAILURE );
		waitpid( pid, &status, 0);
		return WEXITSTATUS( status );
	}

	return true;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	int status,fd[2],rc;
	pid_t pid;
	rc = pipe(fd);
	/* verificare conditie de eroare */
	DIE(rc < 0, "Error when creating pipe");

	pid=fork();
	/* verificare conditie de eroare */
	DIE(pid < 0, "Error on fork");

	if (pid == 0) {
			/* parsez comanda pt copil */
			rc = dup2(fd[1],1);
			DIE(rc<0,"Error on dup2 for child");
			rc = close(fd[0]);
			DIE(rc<0,"Error on closing input for child");
			parse_command(cmd1,level+1,father);
			exit(EXIT_SUCCESS);
	}

	else {
			/* parsez comanda pt parinte */
			dup2( fd[0], 0);
			DIE(rc<0,"Error on dup2 for parent");
			rc = close( fd[1] );
			DIE(rc<0,"Error on closing input for parent");
			parse_command( cmd2, level+1, father);
			waitpid( pid, &status, 0);
			return WEXITSTATUS( status );
	}

	return true; 
}

/**
 * Parse and execute a command.
 */
int parse_command( command_t *c, int level, command_t *father)
{
	int rc=0;

	if (c->op == OP_NONE) {
		/*  execute a simple command */
		rc=parse_simple( c->scmd, level+1, c);
		return rc;
	}

	switch (c->op) {
		case OP_SEQUENTIAL:
			/*  execute the commands one after the other */
			rc = parse_command( c->cmd1, level+1, c);
			rc = parse_command( c->cmd2, level+1, c);
			return rc;
			break;

		case OP_PARALLEL:
			/*  execute the commands simultaneously */
			do_in_parallel( c->cmd1, c->cmd2, level+1, c);
			break;

		case OP_CONDITIONAL_NZERO:
			/*  execute the second command only if the first one
			 * returns non zero */
			rc = parse_command( c->cmd1, level+1,c);
			if(rc != 0)
				rc = parse_command( c->cmd2, level+1, c);
			return rc;
			break;

		case OP_CONDITIONAL_ZERO:
			/*  execute the second command only if the first one
			 * returns zero */
			rc = parse_command(c->cmd1, level+1, c);
			if(rc == 0)
				rc = parse_command( c->cmd2, level+1, c);
			return rc;
			break;

		case OP_PIPE:
			/*  redirect the output of the first command to the
			 * input of the second */
			rc=do_on_pipe( c->cmd1, c->cmd2, level+1, c);
			return rc;
			break;

		default:
			assert(false);
	}

	return rc; /*  replace with actual exit code of command */
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

