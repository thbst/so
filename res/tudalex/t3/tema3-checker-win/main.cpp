/*
 * main.c++
 *
 *  Created on: Apr 23, 2013
 *      Author: tudalex
 */

#include <stdlib.h>
#include <map>
#include <vector>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include "_test/vmsim.h"
#include "_test/debug.h"
#include "_test/util.h"

struct alloc {
	std::vector < char > dirty; //0 -none, 1 -read, 2 -write
	std::vector < w_size_t > map;
	std::vector < w_size_t > page;
	std::vector < char > swapped; // 0 -none, 1 -blanked, 2 -swapped

	vm_map_t mapping;
	w_size_t num_frames;
	w_size_t num_pages;
	w_size_t page_cnt;
};

std::map<w_ptr_t, alloc> mem_map;

static int unmap_page(alloc &t, w_size_t page) {
	w_size_t page_size = w_get_page_size();
	w_ptr_t page_addr = (char*)t.mapping.start+(page*page_size);
	if (t.file[page] != 
}

static void swap_out_page(alloc &t, w_size_t page) {

	
	w_ptr_t page_addr = (char*)t.mapping.start+(page*page_size);
	int rc;

	if (t.dirty[page] == 2 || t.swapped[page] == 1 ) {

		//Daca e drity sau nu a fost swap-uita trebuie sa o swap-ui
		rc = w_set_file_pointer(t.mapping.swap_handle, page*page_size);
		DIE(rc == -1, "lseek");
		rc = w_write_file(t.mapping.swap_handle, page_addr, page_size);
		DIE(rc == -1, "write");
		t.swapped[page] = 2;

	}

	rc = munmap(page_addr,page_size);
	DIE(rc == -1, "munmap");
	MapViewOfFileEx(t.mapping.swap_handle, 

	t.dirty[page] = 0;
}

// Functie ce face rost de o pagina de ram. Daca toate sunt ocupate scot una
// aleator si o swap-ui
static w_size_t get_page(alloc &t) {
	dlog(LOG_DEBUG,"page_cnt %lu zone %p frames %lu\n",
		 t.page_cnt, t.mapping.start, t.num_frames);

	if (t.page_cnt < t.num_frames)
		return t.page_cnt++;
	else {
		size_t r = rand()%t.num_frames;
		swap_out_page(t, t.map[r]);
		t.map[r] = -1;
		return r;
	}

}


void handler(int signum, siginfo_t *info, void *context) {

	w_size_t page_size = w_get_page_size();
	w_ptr_t page_addr = ((w_ptr_t*)info->si_addr);
	int rc;

	dlog(LOG_DEBUG,"Memory location %p", page_addr);
	page_addr = (char*)page_addr - (w_size_t)page_addr%page_size;

	dlog(LOG_DEBUG," page address %p page size %lu\n",page_addr, page_size);


	//Caut alocarea de memorie
	for (std::map<w_ptr_t, alloc>::iterator it = mem_map.begin();
		 it != mem_map.end();
		 ++it) {
		if (page_addr >= it->first &&
			page_addr < (char*)it->first+(page_size*it->second.num_pages)){

			//Am gasit alocarea de memorie

			alloc &t = it->second;
			w_size_t page = ((char*)page_addr-(char*)t.mapping.start)/page_size;
			dlog(LOG_DEBUG,
				"Found it to be a part of %p with dirty set to %u page %lu\n",
				it->first, t.dirty[page], page);

			// Daca pagina e marcata pentru read atunci inseamna ca are nevoie
			// sa scrie in ea
			if (t.dirty[page] == 1) {
				rc = w_protect_mapping(page_addr, 1, PROTECTION_WRITE);
				DIE(rc == -1, "protect");
				t.dirty[page] = 2;
				return;
			}

			// Swap in
			if (t.dirty[page] == 0) {
				size_t frame = get_page(t);
				rc = munmap(page_addr, page_size);
				DIE(rc == -1, "munmap");
				w_ptr_t rp = mmap(page_addr,
								  page_size,
								  PROT_READ|PROT_WRITE,
								  MAP_FIXED|MAP_SHARED,
								  t.mapping.ram_handle,
								  frame*page_size);
				DIE(rp == MAP_FAILED, "mmap");
				rc = w_set_file_pointer(t.mapping.swap_handle, page*page_size);
				DIE(rc == -1, "lseek");

				rc = w_read_file(t.mapping.swap_handle, page_addr, page_size);
				DIE(rc == -1, "read");

				if (!t.swapped[page]) {
					memset(page_addr, 0, page_size);
					t.swapped[page] = 1;
				}
				w_protect_mapping(page_addr, 1, PROTECTION_READ);
				t.dirty[page] = 1;
				t.map[frame] = page;
				return;
			}

		}
	}
}


w_boolean_t vmsim_init(void) {
	return w_set_exception_handler(handler);
}

w_boolean_t vmsim_cleanup(void) {
	//TODO: This thing is not implemented
	w_exception_handler_t t;
	w_get_previous_exception_handler(&t);
	return w_set_exception_handler(t);
}

w_boolean_t vm_alloc(w_size_t num_pages, w_size_t num_frames, vm_map_t *map) {

	if (num_frames > num_pages)
		return FALSE;
	alloc t;
	int rc;

	char template_ram[] = "ramXXXXXX";
	map->ram_handle = mkstemp(template_ram);
	DIE(map->ram_handle == -1, "Creating ram file");
	rc = ftruncate(map->ram_handle, w_get_page_size()*num_frames);
	DIE(rc == -1,"Resize ram file.");

	char template_swap[] = "swapXXXXXX";
	map->swap_handle = mkstemp(template_swap);
	DIE(map->swap_handle == -1, "Creating swap");
	rc = ftruncate(map->swap_handle, w_get_page_size()*num_pages);
	DIE(rc == -1,"Resize swap file.");

	map->start = mmap(NULL,
					  w_get_page_size()*num_pages,
					  PROT_NONE,
					  MAP_SHARED|MAP_ANONYMOUS,
					  -1,
					  0);
	DIE(map->start == MAP_FAILED, "Die mmap");

	t.mapping = *map;
	t.num_frames = num_frames;
	t.num_pages = num_pages;
	t.dirty.resize(num_pages, 0);
	t.page.resize(num_pages, -1);
	t.map.resize(num_frames, -1);
	t.swapped.resize(num_pages, 0);
	t.page_cnt = 0;
	mem_map[t.mapping.start] = t;

	return TRUE;
}

w_boolean_t vm_free(w_ptr_t start) {
	if (mem_map.find(start) == mem_map.end())
		return FALSE;
	alloc t = mem_map[start];
	int rc;
	rc = close(t.mapping.ram_handle);
	DIE(rc == -1,"close");
	rc = close(t.mapping.swap_handle);
	DIE(rc == -1, "close");
	rc = munmap(t.mapping.start, w_get_page_size()*t.num_pages);
	DIE(rc == -1, "munmap");
	mem_map.erase(start);

	return TRUE;
}
