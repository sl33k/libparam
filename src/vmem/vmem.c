/*
 * vmem.c
 *
 *  Created on: Sep 22, 2016
 *      Author: johan
 */

#include <stdio.h>
#include <string.h>
#include <csp/csp.h>

#include <vmem/vmem.h>

extern int __start_vmem, __stop_vmem;

void * vmem_memcpy(void * to, void * from, size_t size) {

	for(vmem_t * vmem = (vmem_t *) &__start_vmem; vmem < (vmem_t *) &__stop_vmem; vmem++) {

		/* Write to VMEM */
		if ((to >= vmem->vaddr) && (to + size <= vmem->vaddr + vmem->size)) {
			//printf("Write to vmem %s, to %p from %p\n", vmem->name, to, from);
			vmem->write(vmem, to - vmem->vaddr, from, size);
			return NULL;
		}

		/* Read */
		if ((from >= vmem->vaddr) && (from + size <= vmem->vaddr + vmem->size)) {
			//printf("Read from vmem %s\n", vmem->name);
			vmem->read(vmem, from - vmem->vaddr, to, size);
			return NULL;
		}

	}

	/* If not vmem found */
	return memcpy(to, from, size);

}

vmem_t * vmem_index_to_ptr(int idx) {
	return ((vmem_t *) &__start_vmem) + idx;
}

int vmem_ptr_to_index(vmem_t * vmem) {
	return vmem - (vmem_t *) &__start_vmem;
}

/* we use __end as the starting point for vaddr selection, since this will never interfere with addresses that are valid within the image */
/* TODO: make sure they also don't interfere with IO mappings. */
extern int _end;
static void * vmem_next_vaddr_ptr = NULL;

void vmem_init(void) {
	for(vmem_t * vmem = (vmem_t *) &__start_vmem; vmem < (vmem_t *) &__stop_vmem; vmem++) {
        if(vmem->init != NULL) {
            vmem->init(vmem);
        }
        /* assign vaddr if not done statically */
        if(vmem->vaddr == NULL) {
            if(vmem_next_vaddr_ptr == NULL) {
                vmem_next_vaddr_ptr = (void *) &_end;
            }
            vmem->vaddr = vmem_next_vaddr_ptr;
            vmem_next_vaddr_ptr += vmem->size;
        }
    }
}

