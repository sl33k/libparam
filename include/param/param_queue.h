/*
 * param_queue.h
 *
 *  Created on: Nov 9, 2017
 *      Author: johan
 */

#ifndef LIB_PARAM_SRC_PARAM_PARAM_QUEUE_H_
#define LIB_PARAM_SRC_PARAM_PARAM_QUEUE_H_

#include <param/param.h>
#include <param/param_list.h>
#include <mpack/mpack.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	PARAM_QUEUE_TYPE_GET,
	PARAM_QUEUE_TYPE_SET,
} param_queue_type_e;

struct param_queue_s {
	char *buffer;
	int buffer_internal;
	int buffer_size;
	param_queue_type_e type;
	int used;
	int version;

	/* State used by serializer */
	uint16_t last_node;
	uint32_t last_timestamp;
	uint16_t last_timediff_ms;

};

typedef struct param_queue_s param_queue_t;

void param_queue_init(param_queue_t * queue, void * buffer, int buffer_size, int used, param_queue_type_e type, int version);

int param_queue_add(param_queue_t *queue, param_t *param, int offset, void *value);
int param_queue_apply(param_queue_t *queue);

void param_queue_print(param_queue_t *queue);
void param_queue_print_local(param_queue_t *queue);

typedef int (*param_queue_callback_f)(void * context, param_queue_t *queue, param_t * param, int offset, void *reader);
int param_queue_foreach(param_queue_t *queue, param_queue_callback_f callback, void * context);


void param_deserialize_id(mpack_reader_t *reader, int *id, int *node, int *offset, param_queue_t *queue);

#define PARAM_QUEUE_FOREACH(param, reader, queue, offset) \
	mpack_reader_t reader; \
	mpack_reader_init_data(&reader, queue->buffer, queue->used); \
	while(reader.left > 0) { \
		int id, node, offset = -1; \
		param_deserialize_id(&reader, &id, &node, &offset, queue); \
		param_t * param = param_list_find_id(node, id); \


#ifdef __cplusplus
}
#endif
#endif /* LIB_PARAM_SRC_PARAM_PARAM_QUEUE_H_ */
