/*
 * rparam.h
 *
 *  Created on: Oct 8, 2016
 *      Author: johan
 */

#ifndef LIB_PARAM_INCLUDE_PARAM_PARAM_SERVER_H_
#define LIB_PARAM_INCLUDE_PARAM_PARAM_SERVER_H_

#include <csp/arch/csp_thread.h>

#define PARAM_SERVER_MTU 200
#define PARAM_PORT_SERVER 10
#define PARAM_PORT_LIST	12

#ifdef __cplusplus
extern "C" {
#endif

/**
 * First byte on all packets is the packet type
 */
typedef enum {

    /* V1 */
	PARAM_PULL_REQUEST 	= 0,
	PARAM_PULL_RESPONSE = 1,
	PARAM_PUSH_REQUEST 	= 2,
	PARAM_PUSH_RESPONSE = 3,        // Push response is an ack, and is same for v1 and v2.
	PARAM_PULL_ALL_REQUEST = 4,     // Pull all request is same for v1 and v2.

	/* V2 */
	PARAM_PULL_REQUEST_V2  = 5,
    PARAM_PULL_RESPONSE_V2 = 6,
    PARAM_PUSH_REQUEST_V2  = 7,
    PARAM_PULL_ALL_REQUEST_V2 = 8,

} param_packet_type_e;

/**
 * Second byte on all packets contains flags
 */
#define PARAM_FLAG_END (1 << 7)

/**
 * Handle incoming parameter requests
 *
 * These are stateless no task or csp connection needed
 *
 * @param packet
 */
void param_serve(csp_packet_t * packet);

#ifdef __cplusplus
}
#endif
#endif /* LIB_PARAM_INCLUDE_PARAM_PARAM_SERVER_H_ */
