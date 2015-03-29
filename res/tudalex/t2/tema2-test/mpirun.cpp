#include <stdio.h>
#include <stdlib.h>


#include "mpi_err.h"
#include "mpi.h"

#include <Windows.h>
#include <vector>
#include <string>

#include <assert.h>

std::vector <HANDLE> v;
int main(int argc, char *argv[]) {
	int size, i, context, bRes;
	DWORD ret;

	
	if (argc < 3) {
		fprintf(stderr, "Usage: mpirun -np N EXE\n");
	}

	sscanf(argv[2], "%d", &size);
	context = rand()%1000;

	// Pornesc procesele

	for (i = 0; i < size; ++i) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

		sprintf(argv[1], "%d", i);
		sprintf(argv[0], "m%d", context);

		// Concatenez parametrii
		std::string acum(argv[3]);
		acum+=" ";
		for (int j = 0; j < argc; ++j)
			acum+=std::string(argv[j])+" ";

		char args[1024];
		sprintf(args,"%s", acum.c_str());
		fprintf(stderr, "%s\n",args); 

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
			fprintf(stderr,"Execution failed for '%s'\n", argv[3]);
			fflush(stderr);
		} else
			v.push_back(pi.hProcess);
		
	}

	// Astept dupa ele si afisez return code-urile sau semnalele
	while (!v.empty()) {
		ret = WaitForMultipleObjects(v.size(),&v[0], false, INFINITE);
		DIE(ret == WAIT_FAILED, "Wait failed.");
		CloseHandle(v[ret]);
		v.erase(v.begin()+ret);
	}
	
	

	return 0;
}
