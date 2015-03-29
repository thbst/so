/* Badoiu Simona Andreea Tema 3 SO - Memorie Virtuala */

#ifndef VIRT_MEM_H_
#define VIRT_MEM_H_	1

#include "common.h"
#include "vmsim.h"

enum page_state {
	STATE_IN_RAM,
	STATE_IN_SWAP,
	STATE_NOT_ALLOC
};
 
struct frame;
 
/* handle pages (virtual pages) */
struct page_table_entry {
	enum page_state state;
	enum page_state prev_state;
	w_boolean_t dirty;
	w_prot_t protection;
	w_ptr_t start;
	w_size_t is_in_swap;
	struct frame *frame;	/* NULL in case page is not mapped */
};
 
/* handle frames (physical pages) */
struct frame {
	w_size_t index;
	char* p;
	/* "backlink" to page_table_entry; NULL in case of free frame */
	struct page_table_entry *pte;
};


w_boolean_t move_to_swap(struct page_table_entry *curr_page,
						w_size_t offset,
						int i,
						w_handle_t fd_swap,
						w_handle_t fd_ram);
w_boolean_t vmsim_give_prot_read(struct page_table_entry *curr_page,
							w_handle_t fd_ram,
							int i);
void vmsim_give_prot_write(struct page_table_entry *curr_page,
							w_handle_t fd_ram, int i);

#endif
