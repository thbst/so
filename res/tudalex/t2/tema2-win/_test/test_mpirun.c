#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	printf("hello\n");
	fprintf(stderr, "rank %s size %s comm %s\n", argv[1], argv[2], argv[0]);
	while (1)
		Sleep(10000);

	return 1;
}
