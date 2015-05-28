/**
  * SO, 2011
  * Lab #7
  *
  * Task #7, lin
  *
  * Page locking
  */
#include <unistd.h> 
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

#include "utils.h"

#define DATA_SIZE 10*2048

static void wait_for_input(const char *msg)
{
	char buf[32];

	printf(" * %s\n", msg);
	printf(" -- Press ENTER to continue ..."); fflush(stdout);
	fgets(buf, 32, stdin);
}


/* lock memory interval [addr, addr + size] */
static void lock_memory(char *addr, size_t  size)
{
	unsigned long page_offset, pagesize;
	int rc;

	pagesize = getpagesize();
	page_offset = (unsigned long) addr%pagesize;

	/* TODO - align addr to page offset and adjust offset*/
	addr -= page_offset;
	size += page_offset;

	/* TODO - lock memory */
	rc = mlock(addr, size);
	DIE(rc == -1, "mlock");
}

/* unlock memory interval [addr, addr + size] */
static void unlock_memory(char *addr, size_t  size)
{
	unsigned long page_offset, pagesize;
	int rc;

	pagesize = getpagesize();
	page_offset = (unsigned long) addr%pagesize;

	/* TODO - align addr to page offset */
	addr -= page_offset;
	size += page_offset;

	/* TODO - unlock memory */
	rc = munlock(addr, size);
	DIE(rc == -1, "munlock");
}

int main(void)
{
	char data[DATA_SIZE];
	char text[] = "azur";

	wait_for_input("beginning");

	lock_memory(data, DATA_SIZE);
	wait_for_input("memory locked ");

	/*  perform real-time data changes */
	memcpy(data, text, sizeof(text));
	printf("data=%s\n",data);
	wait_for_input("accessed memory");

	unlock_memory(data, DATA_SIZE);
	wait_for_input("memory unlocked");

	return 0;
}
