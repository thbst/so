/* Badoiu Simona Andreea - Tema 3 SO - Memorie virtuala */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include "virtualMem.h"
using namespace std;

static vector<vector<struct page_table_entry> > pages;
static vector<w_handle_t> ram_fd;
static vector<char*> name_ram;
static vector<w_handle_t> swap_fd;
static vector<char*> name_swap;
/* index will count how many times vm_alloc was called */
static int ind = 0;
static vector<w_size_t> ram_offset;
static vector<w_size_t> swap_offset;
static vector<w_size_t> nr_frames;
/* keep the index of the page mapped in the frame with that offset */
static vector<vector<int> > frame_page;
static vector<int> ram_full;

FUNC_DECL_PREFIX w_boolean_t vmsim_init(void)
{
	w_boolean_t ok;
	ok = w_set_exception_handler(vmsim_exception_handler);
	if (!ok)
		return FALSE;
	return TRUE;
}

FUNC_DECL_PREFIX w_boolean_t vmsim_cleanup(void)
{
	w_boolean_t ok;
	w_exception_handler_t prev_handler;
	
	ok = w_get_previous_exception_handler(&prev_handler);
	if (ok == FALSE)
		return FALSE;
		
	ok = w_set_exception_handler(prev_handler);
	if (ok == FALSE)
		return FALSE;

	return TRUE;
}

/*
 * allocate physical pages in RAM mapped by virt_pages in swap
 * map is to be filled with start address and handle to RAM and swap
 * files
 */

FUNC_DECL_PREFIX w_boolean_t vm_alloc(w_size_t num_pages, w_size_t num_frames, vm_map_t *map)
{
	char* ram_file_name;
	char* swap_file_name;
	int ram_name_len, swap_name_len, rc, fd_ram, fd_swap;
	w_size_t i;
	w_ptr_t start;
	vector<struct page_table_entry> page_table;
	
	/* sanity checks */
	if (num_frames > num_pages)
		return FALSE;
	
	nr_frames.push_back(num_frames);
	/* init names for RAM and SWAP temp files */
	ram_name_len = 9;
	swap_name_len = 10;
	
	ram_file_name = (char*)malloc((ram_name_len + 1)*sizeof(char));
	if (!ram_file_name)
		return FALSE;
		
	swap_file_name = (char*)malloc((swap_name_len + 1)*sizeof(char));
	if (!swap_file_name) {
		free(ram_file_name);
		return FALSE;
	}
	
	snprintf(ram_file_name, ram_name_len + 1, "%s", "ramXXXXXX");
	snprintf(swap_file_name, swap_name_len + 1, "%s", "swapXXXXXX");
	
	/* open ram and swap temp files */
	fd_ram = mkstemp(ram_file_name);
	if (fd_ram == -1) {
		free(ram_file_name);
		free(swap_file_name);
		return FALSE;
	}
	
	fd_swap = mkstemp(swap_file_name);
	if (fd_swap == -1) {
		free(ram_file_name);
		free(swap_file_name);
		close(fd_ram);
		return FALSE;
	}
	
	/* trunc files to specified dimension */
	rc = ftruncate(fd_ram, num_frames*w_get_page_size());
	if (rc != 0) {
		free(ram_file_name);
		free(swap_file_name);
		close(fd_ram);
		close(fd_swap);
		return FALSE;
	}
	
	rc = ftruncate(fd_swap, num_pages*w_get_page_size());
	if (rc != 0) {
		free(ram_file_name);
		free(swap_file_name);
		close(fd_ram);
		close(fd_swap);
		return FALSE;
	}
	
	/* map num_pages for virtual memory */
	start = mmap(NULL,
				num_pages*w_get_page_size(),
				PROT_NONE,
				MAP_SHARED | MAP_ANONYMOUS,
				0,
				0);
	if (start == (char*) - 1) {
		free(ram_file_name);
		free(swap_file_name);
		close(fd_ram);
		close(fd_swap);
		return FALSE;
	}
	
	for (i = 0; i < num_pages; i++) {
		struct page_table_entry pte;
		pte.state = STATE_NOT_ALLOC;
		pte.prev_state = STATE_NOT_ALLOC;
		pte.dirty = FALSE;
		pte.protection = PROTECTION_NONE;
		pte.start = (char*)start + i*w_get_page_size();
		pte.is_in_swap = 0;
		pte.frame = NULL;
		
		page_table.push_back(pte);
	}
	pages.push_back(page_table);
	
	map->start = start;
	map->ram_handle = fd_ram;
	map->swap_handle = fd_swap;
	
	ram_fd.push_back(fd_ram);
	swap_fd.push_back(fd_swap);
	name_ram.push_back(ram_file_name);
	name_swap.push_back(swap_file_name);
	ram_offset.push_back(0);
	swap_offset.push_back(0);
	ram_full.push_back(0);
	
	ind = ind+1;
	
	return TRUE;
}

/*
 * free space previously allocated with vm_alloc
 * start is the start address of the previously allocated area
 *
 * implementation has to also close handles corresponding to RAM and
 * swap files
 */

FUNC_DECL_PREFIX w_boolean_t vm_free(w_ptr_t start)
{
	int i, rc;
	w_size_t n;
	
	if (start == NULL)
		return FALSE;

	for (i = 0; i < ind; i++) {
		if (start == pages[i][0].start) {
			break;
		}
	}
	
	rc = unlink(name_ram[i]);
	if (rc < 0)
		return FALSE;
	rc = unlink(name_swap[i]);
	if (rc < 0)
		return FALSE;	
	rc = close(ram_fd[i]);
	if (rc < 0)
		return FALSE;
	rc = close(swap_fd[i]);
		if (rc < 0)
			return FALSE;
	
	rc = munmap(start, pages[i].size()*w_get_page_size());
	if (rc == -1) {
		return FALSE;
	}
			
	/* delete elements from vectors after free */
	ram_fd.erase(ram_fd.begin() + i);
	swap_fd.erase(swap_fd.begin() + i);
	/* free struct frame elements */
	for (n = 0; n < pages[i].size(); n++) {
		if (pages[i][n].frame != NULL) {
			free(pages[i][n].frame);
			pages[i][n].frame = NULL;
		}
	}
	pages[i].erase(pages[i].begin(), pages[i].end());
	pages.erase(pages.begin() + i);
	name_swap.erase(name_swap.begin() + i);
	name_ram.erase(name_ram.begin() + i);
	ram_offset.erase(ram_offset.begin() + i);
	nr_frames.erase(nr_frames.begin() + i);
	ram_full.erase(ram_full.begin() + i);
	ind = ind - 1;

	return TRUE;
}

void increase_ram_offset(w_size_t *offset, int *ram_full, int i)
{
	if ((*offset)+1 == nr_frames[i]) {
	 	*ram_full = 1;
	 	*offset = 0;
	 }
	 else {
	 	(*offset)++;
	 }
}

/* get the index of the paged to be swaped */
w_size_t get_page_index(int i, w_size_t ramoffset)
{
	w_size_t aux;
	for (aux = 0; aux < pages[i].size(); aux++) {
		if(pages[i][aux].state == STATE_IN_RAM) {
			//fprintf(stderr, "index %i\n",pages[i][ind].frame->index);
			if ((pages[i][aux].frame)->index == ramoffset);
				return aux;
		}
	}
	return -1;
}

w_boolean_t back_from_swap(struct page_table_entry *curr_page,
							w_size_t ramoffset,
							int i,
							w_handle_t fd_swap,
							w_handle_t fd_ram)
{
	char *buffer, *p;
	buffer = (char*)malloc(w_get_page_size());
	if (!buffer)
		return FALSE;

	mprotect(curr_page->start, w_get_page_size(), PROT_READ | PROT_WRITE);
	memcpy(buffer, curr_page->start, w_get_page_size());
	/* unmap from swap */
	munmap(curr_page->start, w_get_page_size());
	
	p = (char*)mmap(curr_page->start,
					w_get_page_size(),
					PROT_READ | PROT_WRITE,
					MAP_ANONYMOUS | MAP_SHARED | MAP_FIXED,
					fd_ram,
					ramoffset*w_get_page_size());
	if (p == (char*) - 1)
		return FALSE;
	
	//memcpy(curr_page->start, buffer, w_get_page_size());
	w_set_file_pointer(fd_ram, ram_offset[i]*w_get_page_size());
	w_write_file(fd_ram, buffer, w_get_page_size());
	
	mprotect(curr_page->start, w_get_page_size(), PROT_NONE);
	
	curr_page->protection = PROTECTION_NONE;
	curr_page->prev_state = curr_page->state;
	curr_page->state = STATE_IN_RAM;
	curr_page->dirty = FALSE;
	curr_page->frame = (struct frame*)malloc(w_get_page_size());
	curr_page->frame->index = ramoffset;
	curr_page->frame->p = p;
	curr_page->frame->pte = curr_page;
	free(buffer);
	return TRUE;
}

w_boolean_t move_to_swap(struct page_table_entry *curr_page,
						w_size_t offset,
						int i,
						w_handle_t fd_swap,
						w_handle_t fd_ram)
{
	char *buffer, *p, *buffer1;
	int rc;
	
	buffer = (char*)malloc(w_get_page_size());
	if (!buffer)
		return FALSE;

	buffer1 = (char*)malloc(w_get_page_size());
	memset(buffer1, 0, w_get_page_size());
	if (!buffer1)
		return FALSE;
	/* copy from ram */
	mprotect(curr_page->start, w_get_page_size(), PROT_READ | PROT_WRITE);
	memcpy(buffer, curr_page->start, w_get_page_size());
	
	/* write 0 in ram */
	w_set_file_pointer(fd_ram, ram_offset[i]*w_get_page_size());
	w_write_file(fd_ram, buffer1, w_get_page_size());
	
	/*unmap the memory from ram file */
	rc = munmap(curr_page->start, w_get_page_size());
	if (rc == -1) {
		return FALSE;
	}
	
	/*map the memory in swap file if it was never swapped before */
	p = (char*)mmap(curr_page->start,
					w_get_page_size(),
					PROT_READ | PROT_WRITE,
					MAP_ANONYMOUS | MAP_SHARED | MAP_FIXED,
					fd_swap,
					offset*w_get_page_size());
	if (p == (char*) - 1) {
		return FALSE;
	}
	//curr_page->is_in_swap = 1;
	if (curr_page->is_in_swap == 0) {
		curr_page->is_in_swap = 1;
		w_set_file_pointer(fd_swap, offset*w_get_page_size());
		w_write_file(fd_swap, buffer, w_get_page_size());
		mprotect(curr_page->start, w_get_page_size(), PROT_NONE);
	}
	else {
		/* if the page is dirty, copy */
		if (curr_page->dirty == TRUE) {
			mprotect(curr_page->start, w_get_page_size(), PROT_READ | PROT_WRITE);
			w_set_file_pointer(fd_swap, offset*w_get_page_size());
			w_write_file(fd_swap, buffer, w_get_page_size());
		}
		mprotect(curr_page->start, w_get_page_size(), PROT_NONE);
	}

	curr_page->prev_state = curr_page->state;
	curr_page->state = STATE_IN_SWAP;
	curr_page->protection = PROTECTION_NONE;
	curr_page->dirty = FALSE;
	free(curr_page->frame);	
	curr_page->frame = NULL;
	free(buffer);
	free(buffer1);
	return TRUE;
}

/* if curr_page->is_in_swap is 0, then map in ram and give PROT_READ
 * if the page is in swap, then copy the content in ram ang give PROT_READ */
w_boolean_t vmsim_give_prot_read(struct page_table_entry *curr_page,
							w_handle_t fd_ram, w_size_t ramoffset, int i)
{
	char *buffer, *p;
	buffer = (char*)malloc(w_get_page_size());
	if (curr_page->is_in_swap == 0)
		memset(buffer, 0, w_get_page_size());
	else {
		mprotect(curr_page->start, w_get_page_size(), PROT_READ | PROT_WRITE);
		memcpy(buffer, curr_page->start, w_get_page_size());
	}	
	
	if (curr_page->is_in_swap == 0)
		munmap(curr_page->start, w_get_page_size());
	
	p = (char*)mmap(curr_page->start,	
	 				w_get_page_size(),	
	 				PROT_READ | PROT_WRITE,			
	 				MAP_SHARED | MAP_FIXED,			
	 				fd_ram,				
	 				ramoffset*w_get_page_size());
	if (p == (char*) - 1) {
	 	return FALSE;
	}		
	
	/* write 0 in memory if this is the first time the page is mapped in ram
	 * if the page is already in swap, then copy from memory */
	w_set_file_pointer(fd_ram, ramoffset*w_get_page_size());
	w_write_file(fd_ram, buffer, w_get_page_size());
	
	mprotect(curr_page->start, w_get_page_size(), PROT_READ);
	
	curr_page->prev_state = curr_page->state;
	curr_page->state = STATE_IN_RAM;
	curr_page->protection = PROTECTION_READ; 	
	curr_page->frame = (frame*)malloc(sizeof(struct frame));
	if (!curr_page->frame)
		return FALSE;
	curr_page->frame->index = ramoffset;
	curr_page->frame->p = p;
	curr_page->frame->pte = curr_page;
	free(buffer);
	return TRUE;
}

void vmsim_give_prot_write(struct page_table_entry *curr_page,
							w_handle_t fd_ram,
							w_size_t ramoffset,
							int i)
{
	/* give PROT_WRITE to this page */
	mprotect(curr_page->start, w_get_page_size(), PROT_READ | PROT_WRITE);
	
	/* update page_table_entry structure */
	curr_page->dirty = TRUE;
	curr_page->prev_state = curr_page->state;
	curr_page->state = STATE_IN_RAM;
	curr_page->protection = PROTECTION_WRITE;
}

void vmsim_exception_handler(int sig, siginfo_t *siginfo, void *aux)
{
	char* addr;
	int ok, i;
	int curr_page_index = -1, to_swap_index = -1;
	/* start and final addr */
	char *s, *f;
	struct page_table_entry *curr_page;
	w_handle_t fd_ram, fd_swap;
	w_exception_handler_t prev_action;
	
	/* Obtain from siginfo_t the memory location which caused
	 * the page fault */
	addr = (char*)siginfo->si_addr;
	ok = w_get_previous_exception_handler(&prev_action);
	if (ok == FALSE)
		return;
		
	/* check if the signal is SIGSEGV */
	if (sig != SIGSEGV) {
		prev_action(sig, siginfo, aux);
		return;
	}
	 
	/* compute the page number which caused the page fault*/
	for (i = 0; i < ind; i++) {
		s = (char*)((pages[i][0]).start);
		f = (char*)((pages[i][pages[i].size() - 1]).start) +
					w_get_page_size();
		if ((addr - s >= 0) && (f - addr > 0)) {
	 		curr_page_index = (addr - s)/w_get_page_size();
	 		break;
		}
	}
	
	/*if the address is not part of our virtual memory */
	if (curr_page_index < 0) {
		prev_action(sig, siginfo, aux);
		return;
	}
	
	curr_page = &(pages[i][curr_page_index]);
	fd_ram = ram_fd[i];
	fd_swap = swap_fd[i];
	
	/* give the right protection to the page, and move to swap if
	 * necessary */
	if (curr_page->state == STATE_NOT_ALLOC) {
	 	if (ram_full[i] == 0) {
	 		vmsim_give_prot_read(curr_page, fd_ram, ram_offset[i], i);
	 		increase_ram_offset(&(ram_offset[i]), &(ram_full[i]), i);
	 		//return;
	 	}
	 	else {
	 		/* move page to swap and map the other page in memory */
	 		to_swap_index = get_page_index(i, ram_offset[i]);
	 		move_to_swap(&(pages[i][to_swap_index]),
	 						to_swap_index,
	 						i,
	 						fd_swap,
	 						fd_ram);
	 		/*map the new memory on this position in ram */
	 		vmsim_give_prot_read(curr_page, fd_ram, ram_offset[i], i);
	 		increase_ram_offset(&(ram_offset[i]), &(ram_full[i]), i);
	 		//return;
	 	}
	}
	/* modify protection to PROT_WRITE or PROT_READ */
	else {
		if (curr_page->state == STATE_IN_RAM) {
			if (curr_page->protection == PROTECTION_READ) {
				vmsim_give_prot_write(curr_page, fd_ram, ram_offset[i], i);
				return;
			}
			
			if (curr_page->protection == PROTECTION_NONE) {
				vmsim_give_prot_read(curr_page, fd_ram, ram_offset[i], i);
				return;
			}
			prev_action(sig, siginfo, aux);
			return;
		}
		else {
			/* if state is in swap => swap in */
			to_swap_index = get_page_index(i, ram_offset[i]);
	 		/* send to swap */
	 		move_to_swap(&(pages[i][to_swap_index]),
	 						to_swap_index,
	 						i,
	 						fd_swap,
	 						fd_ram);
	 		/* get the page from swap */
	 		back_from_swap(curr_page,
							ram_offset[i],
							i,
							fd_swap,
							fd_ram);
			increase_ram_offset(&(ram_offset[i]), &(ram_full[i]), i);
		}
	}
}
