#include <stdio.h>
#include <stdlib.h>

const char *flowers[] = {
	"rose", "tulip", "daisy"
	"petunia", "orchid", "lily"
};

int main(void)
{
	int i;
	int choice;	

	for (i = 0; i < 25; i++) {
		choice = i % 6;
		printf("%s\n", flowers[choice]);
	}

	return 0;
}
