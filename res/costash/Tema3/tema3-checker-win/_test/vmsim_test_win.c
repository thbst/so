/*
 * virtual machine simulator - useful functions for Windows test suite
 *
 * 2011, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x501	/* vectored exception handler */
#endif

#include <windows.h>

#include "vmsim_test.h"
#include "common.h"
#include "debug.h"

/* count page fault - testing purposes */
static DWORD exception_handler_num_calls = 0;

/*
 * exception handler for testing purpose
 *
 * increments the number of calls and returns; OS will call next
 * vectored handler
 */

LONG vmsim_test_exception_handler(PEXCEPTION_POINTERS info)
{
	EXCEPTION_RECORD *er = info->ExceptionRecord;
	ULONG_PTR addr;

	if (er->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;

	addr = er->ExceptionInformation[1];
	dlog(LOG_LEVEL, "exception address: %p\n", er->ExceptionAddress);

	exception_handler_num_calls++;

	/* OS calls the vmsim exception handler (next in chain) */
	return EXCEPTION_CONTINUE_SEARCH;
}

size_t vmsim_test_get_num_faults(void)
{
	return exception_handler_num_calls;
}

void vmsim_test_set_num_faults(size_t num_faults)
{
	exception_handler_num_calls = num_faults;
}
