/**
 * Operating Systems 2013 - Assignment 1
 *
 */


#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

/* do not use UNICODE */
#undef _UNICODE
#undef UNICODE

#define READ		0
#define WRITE		1

#define MAX_SIZE_ENVIRONMENT_VARIABLE 100

/**
 * Debug method, used by DIE macro.
 */
static VOID PrintLastError(const PCHAR message) {
	CHAR errBuff[1024];

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL,
		GetLastError(),
		0,
		errBuff,
		sizeof(errBuff) - 1,
		NULL);

	fprintf(stderr, "%s: %s\n", message, errBuff);
}

static LPTSTR get_word(word_t *s);

static HANDLE MyOpenFile(PCSTR filename, bool append, bool write) {
	// Functie din laborator un pic modificata
	SECURITY_ATTRIBUTES sa;
	HANDLE hFile;
	DWORD sm; //Share Mode
	DWORD da; //Desired Access
	DWORD cd; //Creation disposition
	DWORD rc;
	
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;
	if (write) {
		da = GENERIC_WRITE;
		sm = FILE_SHARE_WRITE;
		
	} else {
		da = GENERIC_READ;
		sm = FILE_SHARE_READ;
		
	}
	
	// Determinate prin trial & error.
	if (write)
		if (append)
			cd = OPEN_ALWAYS;
		else
			cd = CREATE_ALWAYS;
	else
		cd = OPEN_EXISTING;

	hFile = CreateFile(
		filename,
		da,
		sm,
		&sa,
		cd, 
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DIE(hFile == INVALID_HANDLE_VALUE, "Couldn't open file.");

	if (append) {
		rc = SetFilePointer(hFile, 0, NULL, FILE_END); 
		DIE(rc == INVALID_SET_FILE_POINTER, 
			"Couldn't set file pointer to end of file."); 
	}

	return hFile;
}

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir) {
	BOOL rc;
	LPSTR path = get_word(dir);
	rc = SetCurrentDirectory(path);
	if (!rc) 
		fprintf(stderr, "Couldn't change folder.");
	return 0;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit() {
	return SHELL_EXIT;
}

/**
 * Concatenate parts of the word to obtain the command
 */
static LPTSTR get_word(word_t *s)
{
	DWORD string_length = 0;
	DWORD substring_length = 0;

	LPTSTR string = NULL;
	CHAR substring[MAX_SIZE_ENVIRONMENT_VARIABLE];

	DWORD dwret;

	while (s != NULL) {
		strcpy(substring, s->string);

		if (s->expand == true) {
			dwret = GetEnvironmentVariable(substring, 
										   substring, 
										   MAX_SIZE_ENVIRONMENT_VARIABLE);
			if (!dwret)
				/* Environment Variable does not exist. */
				strcpy(substring, "");
		}

		substring_length = strlen(substring);

		string =(LPSTR) realloc(string, string_length + substring_length + 1);
		memset(string + string_length, 0, substring_length + 1);

		strcat(string, substring);
		string_length += substring_length;

		s = s->next_part;
	}

	return string;
}

/**
 * Parse arguments in order to succesfully process them using CreateProcess
 */
static LPTSTR get_argv(simple_command_t *command)
{
	LPTSTR argv = NULL;
	LPTSTR substring = NULL;
	word_t *param;

	DWORD string_length = 0;
	DWORD substring_length = 0;

	argv = get_word(command->verb);
	assert(argv != NULL);

	string_length = strlen(argv);

	param = command->params;
	while (param != NULL) {
		substring = get_word(param);
		substring_length = strlen(substring);

		argv = (LPSTR) realloc(argv, string_length + substring_length + 4);
		assert(argv != NULL);

		strcat(argv, " ");

		/* Surround parameters with ' ' */
		strcat(argv, "'");
		strcat(argv, substring);
		strcat(argv, "'");

		string_length += substring_length + 3;
		param = param->next_word;

		free(substring);
	}

	return argv;
}

/**
 * Parse and execute a simple command, by either creating a new processing or
 * internally process it.
 */
unsigned long parse_simple(simple_command_t *s, 
						   int wait, 
						   command_t *father, 
						   HANDLE *hInPipe, 
						   HANDLE *hOutPipe, 
						   std::vector<HANDLE> *waitHandles) {
	assert(s!=NULL);
	
	unsigned long rc;
	char *verb;
	char *in;
	char *out;
	char *err;
	char *args;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD dwRes;
	BOOL bRes;
	HANDLE hFile;

	assert(s!= NULL);
	assert(father!=NULL);
	
	verb = get_word(s->verb);
	in = get_word(s->in);
	out = get_word(s->out);
	err = get_word(s->err);
	args = get_argv(s);

	// Fix pentru outputl de la cd
	if (strcmp("cd", verb) == 0) {
		if (out != NULL) {
			SECURITY_ATTRIBUTES sa;
			ZeroMemory(&sa, sizeof(sa));
			hFile = CreateFile(
						out,
						GENERIC_WRITE,
						FILE_SHARE_WRITE,
						&sa,
						CREATE_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL,
						NULL);
		}
		rc = shell_cd(s->params);
		if (in != NULL)
			CloseHandle(hFile);
		goto exit;
	}

	if (strcmp("exit", verb) == 0 || strcmp("quit",verb) == 0) {
		rc = shell_exit();
		goto exit;
	}

	//Setam variabilele de mediu
	if (s->verb->next_part!=NULL &&
		s->verb->next_part->string[0] == '=' &&
		s->verb->next_part->next_part!=NULL) {
		rc = !SetEnvironmentVariable(s->verb->string,
									 s->verb->next_part->next_part->string);
		
		goto exit;
	}

	// Setam informatiile de sistem si de pipe-uri
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESTDHANDLES;
	
	if (hInPipe != NULL)
		si.hStdInput = *hInPipe;
	else if (in != NULL)
		si.hStdInput = MyOpenFile(in,false, false);
	else
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

	if (hOutPipe != NULL)
		si.hStdOutput = *hOutPipe;
	else if (out != NULL)
		si.hStdOutput = MyOpenFile(out, (s->io_flags&IO_OUT_APPEND) != 0, true);
	else
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	if (err != NULL) {
		if (out != NULL && strcmp(err, out) == 0)
			DuplicateHandle(GetCurrentProcess(),
							si.hStdOutput, 
							GetCurrentProcess(), 
							&si.hStdError,
							0, 
							TRUE, 
							DUPLICATE_SAME_ACCESS);
		else
			si.hStdError = MyOpenFile(err, 
									  (s->io_flags&IO_ERR_APPEND) != 0, 
									  true);
	} else
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	bRes = CreateProcess(
			NULL,
			args,
			NULL,
			NULL,
			TRUE,
			0,
			NULL,
			NULL,
			&si,
			&pi);
	if (!bRes) {
		fprintf(stderr,"Execution failed for '%s'\n",verb);
		fflush(stderr);
	}
	
	// Inchidem handle-urile
	if (out!=NULL || hOutPipe!=NULL) {
		CloseHandle(si.hStdOutput);
		fprintf(stderr,"Closing handle %d\n", si.hStdOutput);
	}
	if (in!= NULL || hInPipe!=NULL) {
		CloseHandle(si.hStdInput);
		fprintf(stderr,"Closing handle %d\n", si.hStdInput);
	}
	if (err!=NULL)
		CloseHandle(si.hStdError);
	fflush(stderr);
	if (hOutPipe != NULL)
		free(hOutPipe);
	if (hInPipe != NULL)
		free(hInPipe);

	// Daca s-a reusit 
	if (bRes)
		if (!wait) {
			dwRes = WaitForSingleObject(pi.hProcess, INFINITE);
			DIE(dwRes == WAIT_FAILED, "WaitForSingleObject");

			GetExitCodeProcess(pi.hProcess, &rc);
		
			CloseHandle(pi.hProcess);
		} else {
			rc = -1;
			waitHandles->push_back(pi.hProcess);
		}

exit:
	free(verb);
	free(in);
	free(out);
	free(err);
	free(args);
	return rc;
}

/**
 * Process two commands in parallel, by creating two children.
 */

struct parse_param {
	command_t *cmd;
	command_t *father;
	int wait;
	HANDLE *hInFilePipe;
	HANDLE *hOutFilePipe;
	
};

DWORD WINAPI parse_cmd_wrapper(LPVOID parse_param) {
	DIE(parse_param == NULL, "Recieved null parameter.");
	struct parse_param *data = (struct parse_param*) parse_param;
	DWORD rc, ret;
	std::vector<HANDLE> *v = new std::vector<HANDLE>();
	rc = parse_command(data->cmd, data->wait, data->father, data->hInFilePipe, 
					   data->hOutFilePipe, v);
	while (!v->empty()) {
		ret = WaitForMultipleObjects(v->size(),&((*v)[0]), false, INFINITE);
		DIE(ret == WAIT_FAILED, "Wait failed.");
		CloseHandle((*v)[ret]);
		v->erase(v->begin()+ret);
	}
	free(v);
	free(data);
	return rc;
}

static int do_in_parallel(command_t *cmd1, 
						  command_t *cmd2, 
						  int wait, 
						  command_t *father,
						  HANDLE *hInPipe, 
						  HANDLE *hOutPipe, 
						  std::vector<HANDLE> *v) {
	struct parse_param *data =(struct parse_param*) 
			calloc(1, sizeof(struct parse_param));
	int rc;
	DWORD dwRes;
	HANDLE hThread;
	
	data->cmd = cmd1;
	data->father = father;
	data->wait = wait;
	data->hInFilePipe = hInPipe;
	data->hOutFilePipe = hOutPipe;
	
	hThread = CreateThread(NULL, 0, parse_cmd_wrapper,  data, 0, 0);
	DIE(hThread == NULL, "Couldn't spawn thread.");
	
	rc = parse_command(cmd2, wait, father,  hInPipe, hOutPipe, v);
	
	dwRes = WaitForSingleObject(hThread, INFINITE);
	DIE(dwRes == WAIT_FAILED, "WaitForSingleObject");
	
	return rc;
}

/**
 * Run commands by creating an annonymous pipe (cmd1 | cmd2)
 */


static int do_on_pipe(command_t *cmd1, 
					  command_t *cmd2, 
					  int wait, 
					  command_t *father, 
					  HANDLE *hInPipe, 
					  HANDLE *hOutPipe, 
					  std::vector<HANDLE> *v) {
	assert(cmd1!=NULL);
	assert(cmd2!=NULL);
	
	HANDLE *In = (HANDLE*)calloc(1, sizeof(HANDLE));
	HANDLE *Out = (HANDLE*)calloc(1, sizeof(HANDLE));
	SECURITY_ATTRIBUTES sa, sa_t;
	struct parse_param *data = (struct parse_param*)
		calloc(1, sizeof(struct parse_param));
	
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = true;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(sa);
	
	CreatePipe(In, Out, &sa, 0);

	
	data->cmd = cmd1;
	data->father = father;
	data->wait = wait;
	data->hInFilePipe = hInPipe;
	data->hOutFilePipe = Out;

	ZeroMemory(&sa_t, sizeof(sa_t));
	sa_t.bInheritHandle = true;
	sa_t.lpSecurityDescriptor = NULL;
	
	parse_command(cmd1, 1, father, hInPipe, Out, v);


	return parse_command(cmd2, wait, father, In, hOutPipe, v); 
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, 
				  int wait, 
				  command_t *father, 
				  HANDLE *hInPipe, 
				  HANDLE *hOutPipe, 
				  std::vector<HANDLE> *v) {
	assert(c!=NULL);

	int rc;
	if (c->op == OP_NONE) {
		
		
		return parse_simple(c->scmd, wait, c, hInPipe, hOutPipe, v); 
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		parse_command(c->cmd1, wait, c, hInPipe, hOutPipe, v);
		rc = parse_command(c->cmd2, wait, c, hInPipe, hOutPipe, v);
		break;

	case OP_PARALLEL:
		rc = do_in_parallel(c->cmd1, c->cmd2, wait, c, hInPipe, hOutPipe, v);
		break;

	case OP_CONDITIONAL_NZERO:
		rc = parse_command(c->cmd1, wait, c, hInPipe, hOutPipe, v);
		if (rc != 0)
			rc = parse_command(c->cmd2, wait, c, hInPipe, hOutPipe, v);
		break;

	case OP_CONDITIONAL_ZERO:
		rc = parse_command(c->cmd1, wait, c, hInPipe, hOutPipe, v);
		if (rc == 0)
			rc = parse_command(c->cmd2, wait, c, hInPipe, hOutPipe, v);
		break;

	case OP_PIPE:
		rc = do_on_pipe(c->cmd1, c->cmd2, wait, c, hInPipe, hOutPipe, v);
		break;

	default:
		assert(false);
		break;
	}

	return rc;
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

	chunk =(char*) calloc(CHUNK_SIZE, sizeof(char));
	if (chunk == NULL) {
		fprintf(stderr, ERR_ALLOCATION);
		exit(EXIT_FAILURE);
	}

	instr = NULL;
	instr_length = 0;

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

		instr = (char*)realloc(instr, instr_length + CHUNK_SIZE);
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

