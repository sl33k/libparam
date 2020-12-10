/*
 * param_list.c
 *
 *  Created on: Oct 9, 2016
 *      Author: johan
 */

#include <stdio.h>
#include <malloc.h>

#include <csp/csp.h>
#include <csp/arch/csp_malloc.h>
#include <csp/csp_endian.h>
#include <param/param.h>
#include <param/param_list.h>
#include <param/param_server.h>

#include "../param_string.h"
#include "param_list.h"
#include "libparam.h"

#ifdef PARAM_HAVE_SYS_QUEUE
#include <sys/queue.h>
#endif

/**
 * The storage size (i.e. how closely two param_t structs are packed in memory)
 * varies from platform to platform (in example on x64 and arm32). This macro
 * defines two param_t structs and saves the storage size in a define.
 */
#ifndef PARAM_STORAGE_SIZE
static const param_t param_size_set[2] __attribute__((aligned(1)));
#define PARAM_STORAGE_SIZE ((intptr_t) &param_size_set[1] - (intptr_t) &param_size_set[0])
#endif

#ifdef PARAM_HAVE_SYS_QUEUE
static SLIST_HEAD(param_list_head_s, param_s) param_list_head = {};
#endif

param_t * param_list_iterate(param_list_iterator * iterator) {

	/**
	 * GNU Linker symbols. These will be autogenerate by GCC when using
	 * __attribute__((section("param"))
	 */
	__attribute__((weak)) extern param_t __start_param;
	__attribute__((weak)) extern param_t __stop_param;

	/* First element */
	if (iterator->element == NULL) {

		/* Static */
		if ((&__start_param != NULL) && (&__start_param != &__stop_param)) {
			iterator->phase = 0;
			iterator->element = &__start_param;
		} else {
			iterator->phase = 1;
#ifdef PARAM_HAVE_SYS_QUEUE
			iterator->element = SLIST_FIRST(&param_list_head);
#endif
		}

		return iterator->element;
	}

	/* Static phase */
	if (iterator->phase == 0) {

		/* Increment in static memory */
		iterator->element = (param_t *)(intptr_t)((char *)iterator->element + PARAM_STORAGE_SIZE);

		/* Check if we are still within the bounds of the static memory area */
		if (iterator->element < &__stop_param)
			return iterator->element;

		/* Otherwise, switch to dynamic phase */
		iterator->phase = 1;
#ifdef PARAM_HAVE_SYS_QUEUE
		iterator->element = SLIST_FIRST(&param_list_head);
		return iterator->element;
#else
		return NULL;
#endif
	}

#ifdef PARAM_HAVE_SYS_QUEUE
	/* Dynamic phase */
	if (iterator->phase == 1) {

		iterator->element = SLIST_NEXT(iterator->element, next);
		return iterator->element;
	}
#endif

	return NULL;

}


int param_list_add(param_t * item) {
	if (param_list_find_id(item->node, item->id) != NULL)
		return -1;
#ifdef PARAM_HAVE_SYS_QUEUE
	SLIST_INSERT_HEAD(&param_list_head, item, next);
#else
	return -1;
#endif
	return 0;
}

param_t * param_list_find_id(int node, int id)
{
	if (node == -1)
		node = PARAM_LIST_LOCAL;
	if (node == csp_get_address())
		node = PARAM_LIST_LOCAL;

	param_t * found = NULL;
	param_t * param;
	param_list_iterator i = {};
	while ((param = param_list_iterate(&i)) != NULL) {

		if (param->node != node)
			continue;

		if (param->id == id) {
			found = param;
			break;
		}

		continue;
	}

	return found;
}

param_t * param_list_find_name(int node, char * name)
{
	if (node == -1)
		node = PARAM_LIST_LOCAL;

	param_t * found = NULL;
	param_t * param;
	param_list_iterator i = {};
	while ((param = param_list_iterate(&i)) != NULL) {

		if (param->node != node)
			continue;

		if (strcmp(param->name, name) == 0) {
			found = param;
			break;
		}

		continue;
	}

	return found;
}

void param_list_print(uint32_t mask) {
	param_t * param;
	param_list_iterator i = {};
	while ((param = param_list_iterate(&i)) != NULL) {
		if ((param->mask & mask) || (mask == 0xFFFFFFFF)) {
			param_print(param, -1, NULL, 0, 2);
		}
	}
}

void param_list_download(int node, int timeout, int list_version, int verbose) {

	/* Establish RDP connection */
	csp_conn_t * conn = csp_connect(CSP_PRIO_HIGH, node, PARAM_PORT_LIST, timeout, CSP_O_RDP | CSP_O_CRC32);
	if (conn == NULL)
		return;

	int count = 0;
	csp_packet_t * packet;
	while((packet = csp_read(conn, timeout)) != NULL) {

		//csp_hex_dump("Response", packet->data, packet->length);

		int strlen;
		int addr;
		int id;
		int type;
		int size;
		int mask;
		char * name;

	    if (list_version == 1) {

	        param_transfer_t * new_param = (void *) packet->data;

	        name = new_param->name;
	        strlen = packet->length - offsetof(param_transfer_t, name);
	        addr = csp_ntoh16(new_param->id) >> 11;
            id = csp_ntoh16(new_param->id) & 0x7FF;
            type = new_param->type;
            size = new_param->size;
            mask = csp_ntoh32(new_param->mask) | PM_REMOTE;

	    } else {

	        param_transfer2_t * new_param = (void *) packet->data;

	        name = new_param->name;
	        strlen = packet->length - offsetof(param_transfer2_t, name);
            addr = csp_ntoh16(new_param->node);
            id = csp_ntoh16(new_param->id) & 0x7FF;
            type = new_param->type;
            size = new_param->size;
            mask = csp_ntoh32(new_param->mask) | PM_REMOTE;

	    }

		if (size == 255)
			size = 1;

		param_t * param = param_list_create_remote(id, addr, type, mask, size, name, strlen);
		if (param == NULL) {
			csp_buffer_free(packet);
			break;
		}

        if(verbose) {
            printf("Got param: %s[%d]\n", param->name, param->array_size);
        }

		/* Add to list */
		if (param_list_add(param) != 0)
			param_list_destroy(param);

		csp_buffer_free(packet);
		count++;
	}

    if(verbose) {
        printf("Received %u parameters\n", count);
    }
	csp_close(conn);
}

void param_list_destroy(param_t * param) {
	free(param);
}

param_t * param_list_create_remote(int id, int node, int type, uint32_t mask, int array_size, char * name, int namelen) {

	if (array_size < 1)
		array_size = 1;

	struct param_heap_s {
		param_t param;
		union {
			uint64_t alignme;
			uint8_t buffer[param_typesize(type) * array_size];
		};
		char name[namelen+1];
	} *param_heap = calloc(sizeof(struct param_heap_s), 1);

	param_t * param = &param_heap->param;
	if (param == NULL) {
		return NULL;
	}

	param->vmem = NULL;
	param->name = param_heap->name;
	param->addr = param_heap->buffer;
	param->unit = NULL;

	param->id = id;
	param->node = node;
	param->type = type;
	param->mask = mask;
	param->array_size = array_size;
	param->array_step = param_typesize(type);

	strncpy(param->name, name, namelen);
	param->name[namelen] = '\0';

	//printf("Created %s\n", param->name);

	return param;

}

