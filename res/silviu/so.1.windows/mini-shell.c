#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "parser.h"
#include <process.h>

#define MAX_LINE_SIZE	256
#define strdup			_strdup
#define ERR_MSG		": command not found\n"

const HANDLE SPECIAL_COMMAND_HANDLE = (HANDLE) 1;

void parse_error(const char * str, const int where)
{
	fprintf(stderr, "Parse error near %d: %s\n", where, str);
}

static VOID RedirectHandle(STARTUPINFO *psi, HANDLE in, HANDLE out, HANDLE err)
{
	psi->dwFlags = STARTF_USESTDHANDLES;
	psi->hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	psi->hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	psi->hStdError = GetStdHandle(STD_ERROR_HANDLE);
	
	if (in != INVALID_HANDLE_VALUE)
		psi->hStdInput = in;
	if (out != INVALID_HANDLE_VALUE)
		psi->hStdOutput = out;
	if (err != INVALID_HANDLE_VALUE)
		psi->hStdError = err;
}

static PCHAR ExpandVariable(PCSTR key)
{
	char* buff = NULL;
	int path_size = GetEnvironmentVariable(key, buff, 0);
	if (!path_size) {
		return strdup("");
	}
	buff = malloc(path_size);
	path_size = GetEnvironmentVariable(key, buff, path_size);
	return strdup(buff);
}

static VOID CloseProcess(LPPROCESS_INFORMATION lppi)
{
	CloseHandle(lppi->hProcess);
}


char* concat(char* s1, char* s2)
{
	size_t len1, len2;
	char* ret;
	len1 = strlen(s1);
	len2 = strlen(s2);

	ret = malloc(len1 + len2+ 2);

	memcpy(ret, s1, len1);

	ret[len1] = ' ';

	memcpy(ret + len1 + 1, s2, len2);
	ret[len1 + len2 + 1] = '\0';
	return ret;
}

void append(char** ps1, char* s2)
{
	char* s1 = *ps1;
	char* ret = concat(s1, s2);
	free(s1);
	*ps1 = ret;
}

static PCHAR escape_quotes(PCHAR word)
{
	char* it = word, *new_word, *new_it;
	int nr_quotes = 0;
	while(*it != '\0') {
		if (*it == '"') 
			nr_quotes++;
		it++;
	}

	new_word = calloc(strlen(word) + nr_quotes + 1, 1);
	DIE(!new_word, "Calloc error in escape_quotes");

	new_it = new_word;
	it = word;
	while(*it) {
		if (*it == '"') {
			*new_it = '\\';
			new_it++;
		}

		*new_it = *it; 
		new_it++;
		it++;
	}
	return new_word;
}

char* word_to_string(word_t* wd, BOOL ghil)
{
	char* first_word = strdup(wd->string);
	char* ret;
	DIE(!first_word, "No memory in word_to_string()");

	if (wd->expand) {
		free(first_word);
		first_word = ExpandVariable(wd->string);
	}
	if (ghil) {
		char* tmp;
		ret = calloc(strlen(first_word) + 5, 1); 
		DIE(!ret, "No memory in word_to_string()");

		tmp = first_word;
		first_word = escape_quotes(first_word);
		free(tmp);

		_snprintf_s(ret, strlen(first_word) + 5, strlen(first_word) + 5, "\"%s\"", first_word);
		free(first_word);
	}
	else 
		ret = first_word;
	if (wd->next_word) {
		char* next_word = word_to_string(wd->next_word, ghil);
		DIE(!next_word, "No memory in word_to_string()");
		append(&ret, next_word);
		free(next_word);
	}

	if (wd->next_part) {
		char* next_part = word_to_string(wd->next_part, FALSE);
		char* tmp = malloc(strlen(ret) + strlen(next_part) + 1);
		if (!strcmp(next_part, "="))
			printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");

		memcpy(tmp, ret, strlen(ret));
		memcpy(tmp + strlen(ret), next_part, strlen(next_part) + 1);
		free(next_part);
		ret = tmp;
	}

	return ret;
}

static BOOL change_directory(PCHAR path)
{
	return SetCurrentDirectory(path);
}

static void set_variable(char* command)
{
	BOOL rc;
	char* var, *val;
	char* it = command;
	char* egal = strchr(command, '=');
	int chars = 0, len_var = 0, len_val = 0;
	while (it != egal) {
		chars++;
		it++;
	}
	len_var = chars;
	var = malloc(len_var + 1);
	memcpy(var, command, len_var);
	var[len_var] = '\0';

	len_val = strlen(command) - len_var - 1;
	val = malloc(len_val + 1);
	memcpy(val, command + len_var + 1, len_val + 2);

	//printf("set $[%s][%d]=[%s][%d]\n", var, len_var, val, len_val);
	rc = SetEnvironmentVariable(var, val);
	//DIE(!rc, "setevn");

}

static HANDLE WinCreateProcess(PCHAR command, HANDLE in, HANDLE out, HANDLE err) 
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	BOOL bRes;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	RedirectHandle(&si, in, out, err);
	
	if (!strcmp(command, "exit") || !strcmp(command, "quit"))
		exit(EXIT_SUCCESS);

	bRes = CreateProcess(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (!bRes) {
		int written = 0;
		if (err != INVALID_HANDLE_VALUE) {
			WriteFile(err, command , strlen(command), &written, NULL);
			WriteFile(err, ERR_MSG, strlen(ERR_MSG), &written, NULL);
		}
		else
			fprintf(stderr, "Execution failed for '%s'\n", command);
		return INVALID_HANDLE_VALUE;
	}
	CloseHandle(pi.hThread);
	return pi.hProcess;
}

static VOID seek_end_file(HANDLE file)
{
	SetFilePointer(file, 0, NULL, FILE_END);
}

static HANDLE run_simple_command_nowait(simple_command_t* sc, HANDLE _in, HANDLE _out)
{
	BOOL success = TRUE;
	HANDLE  in  = _in, 
			out = _out, 
			err = INVALID_HANDLE_VALUE;
	HANDLE proc;
	DWORD open_mode;

	char* command = word_to_string(sc->verb, FALSE);

	if (!strcmp(command, "cd")) {
		BOOL rc = TRUE;
		if (sc->params != NULL) {
			char* path = word_to_string(sc->params, FALSE);
			rc = change_directory(path);
			free(path);
		}
		if (rc)
			return SPECIAL_COMMAND_HANDLE;
		return  INVALID_HANDLE_VALUE;
	}


	if (strchr(command, '=')) {
		set_variable(command);
		return SPECIAL_COMMAND_HANDLE;
	}

	if (sc->params != NULL) {
		char* params = word_to_string(sc->params, TRUE);
		append(&command, params);
		free(params);
	}
	
	if (sc->in != NULL) {
		SECURITY_ATTRIBUTES sa;
		char* file_name = word_to_string(sc->in, FALSE);

		ZeroMemory(&sa, sizeof(sa));
		sa.bInheritHandle = TRUE;

		in = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (in == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "ERROR opening input");
			success = FALSE;
		}
		free(file_name);
	}

	if (sc->out != NULL) {
		SECURITY_ATTRIBUTES sa;
		char* file_name = word_to_string(sc->out, FALSE);

		ZeroMemory(&sa, sizeof(sa));
		sa.bInheritHandle = TRUE;
		if (sc->io_flags == IO_OUT_APPEND)
			open_mode = OPEN_ALWAYS;
		else
			open_mode = CREATE_ALWAYS;

		out = CreateFile(file_name, GENERIC_WRITE, FILE_SHARE_WRITE, &sa, open_mode, FILE_ATTRIBUTE_NORMAL, NULL);
		if (open_mode == OPEN_ALWAYS)
			seek_end_file(out);
		if (out == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "ERROR opening output");
			success = FALSE;
		}
		free(file_name);
	}

	if (sc->err != NULL) {
		SECURITY_ATTRIBUTES sa;
		char* file_name;
		if (sc->io_flags == IO_ERR_APPEND)
			open_mode = OPEN_ALWAYS;
		else
			open_mode = CREATE_ALWAYS;

		if (sc->err == sc->out) {
			err = out;//DUPLICATEHANDLE?
		} else {

			file_name  = word_to_string(sc->err, FALSE);
			ZeroMemory(&sa, sizeof(sa));
			sa.bInheritHandle = TRUE;

			err = CreateFile(file_name, GENERIC_WRITE, FILE_SHARE_WRITE, &sa, open_mode, FILE_ATTRIBUTE_NORMAL, NULL);
			if (open_mode == OPEN_ALWAYS)
				seek_end_file(err);
			if (err == INVALID_HANDLE_VALUE) {
				fprintf(stderr, "ERROR opening Error");
				success = FALSE;
			}
			free(file_name);
		}
	}

	if (success)
		proc = WinCreateProcess(command, in, out, err);

	CloseHandle(in);
	CloseHandle(out);
	CloseHandle(err);
	free(command);

	if (!success)
		return INVALID_HANDLE_VALUE;
	return proc;
}

static int run_simple_command(simple_command_t* sc, BOOL wait)
{
	DWORD dwRes;
	HANDLE proc;

	proc = run_simple_command_nowait(sc, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE);
	if (proc == INVALID_HANDLE_VALUE)
		return 1;
	if (proc == SPECIAL_COMMAND_HANDLE)
		return 0;

	if (!wait)
		goto exit;

	dwRes = WaitForSingleObject(proc, INFINITE);
	if (dwRes == WAIT_OBJECT_0) {
		GetExitCodeProcess(proc, &dwRes);
		CloseHandle(proc);
		return dwRes;
	}
exit:
	CloseHandle(proc);
	return 0;
}

static DWORD recursive_wait(command_t* root, BOOL wait)
{
	DWORD dwRes;
	if (root->op == OP_PIPE) {
		recursive_wait(root->cmd1, wait);
		return recursive_wait(root->cmd2, wait);
	}
	if (root->aux == INVALID_HANDLE_VALUE)
		return 1;
	if (root->aux == SPECIAL_COMMAND_HANDLE)
		return 0;
	
	if(!wait)
		goto exit;

	dwRes = WaitForSingleObject(root->aux, INFINITE);
	if (dwRes != WAIT_OBJECT_0)
		return 1;
 
	GetExitCodeProcess(root->aux, &dwRes);
exit:
	CloseHandle(root->aux); 
	return dwRes;
}

static VOID recursive_pipe(command_t* root, HANDLE in, HANDLE out)
{
	BOOL bRes;
	HANDLE hRead, hWrite;
	SECURITY_ATTRIBUTES sa;

	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;

	
	if (root->op == OP_PIPE) {
		bRes = CreatePipe(&hRead, &hWrite,&sa, 0);
		DIE(!bRes, "PIPE CRE");

		recursive_pipe(root->cmd1, in, hWrite);
		CloseHandle(hWrite);
		recursive_pipe(root->cmd2, hRead, out);
		CloseHandle(hRead);
		root->aux = INVALID_HANDLE_VALUE;
		//TODO: return code of cmd1 or cmd2
	}
	else if (root->op == OP_NONE) {
		root->aux = run_simple_command_nowait(root->scmd, in, out);
	}
}

extern command_t* command_deep_copy(command_t* root);
extern void command_deep_free(command_t* root);

static int run_command(command_t* root, BOOL wait);

static void thread_func(void* arg)
{
	command_t* root = (command_t*) arg;
	run_command(root, TRUE);
	command_deep_free(root);
	_endthread();
	}

static VOID run_command_in_parallel(command_t* root)
{
	//HANDLE hthread;
	command_t* root_copy = command_deep_copy(root);
	//hthread =
	_beginthread(thread_func, 0, root_copy);
	//DIE(hthread == INVALID_HANDLE_VALUE, "Cannot create parallel thread");
	//CloseHandle(hthread);
}

static int run_command(command_t* root, BOOL wait)
{
	int ret = 0;
	DWORD dwRes;
	switch(root->op) {
		case OP_NONE:
			return run_simple_command(root->scmd, wait);

		case OP_PIPE:
			recursive_pipe(root, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE);
			dwRes = recursive_wait(root, wait);
			return dwRes;

		case OP_SEQUENTIAL:
			run_command(root->cmd1, TRUE);
			return run_command(root->cmd2, wait);

		case OP_CONDITIONAL_ZERO:
			ret = run_command(root->cmd1, TRUE);
			if (ret)
				return ret;
			return run_command(root->cmd2, wait);

		case OP_CONDITIONAL_NZERO:
			ret = run_command(root->cmd1, TRUE);
			if (!ret)
				return ret;
			return run_command(root->cmd2, wait);

		case OP_PARALLEL:
			//TODO: functie care face un thread in care ruleaza run_command(cmd1)
			run_command_in_parallel(root->cmd1);
			return run_command(root->cmd2, wait);

		default:
			printf("RUN COMMAND case not treated.\n");
			return 0;
	}
}



int main(int argc, char** argv)
{
	CHAR line[MAX_LINE_SIZE];
	BOOL ret;

	while (1) {
		command_t *root = NULL;

		printf("> ");
		fflush(stdout);

		ZeroMemory(line, MAX_LINE_SIZE);

		if (fgets(line, sizeof(line), stdin) == NULL) {
			   exit(EXIT_SUCCESS);
		}

		ret = parse_line(line, &root);
		if (!ret || !root) continue;
		run_command(root, TRUE);
		free_parse_memory();
	}
	
	return 0;
}