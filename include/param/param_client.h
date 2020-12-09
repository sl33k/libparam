/*
 * param_client.h
 *
 *  Created on: Dec 14, 2016
 *      Author: johan
 */

#ifndef LIB_PARAM_INCLUDE_PARAM_PARAM_CLIENT_H_
#define LIB_PARAM_INCLUDE_PARAM_PARAM_CLIENT_H_

#include <param/param.h>
#include <param/param_queue.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SINGLE PARAMETER API
 *
 * The following functions are for getting/setting of single parameters on a remote system.
 * Beneath these functions uses the QUEUE API but it saves some time for the developer
 * when only a single parameter needs to be accessed.
 *
 */

/**
 * PULL single:
 *
 * Executes an immediate parameter fetch of a single parameter from a remote system.
 *
 * @param param pointer to parameter description
 * @param array offset (-1 for all)
 * @param verbose printout when received
 * @param host remote csp node
 * @param timeout in ms
 * @param version 1 or 2
 * @return 0 = ok, -1 on network error
 */
int param_pull_single(param_t *param, int offset, int verbose, int host, int timeout, int version);

/**
 * PULL all
 * @param verbose printout when received
 * @param host remote csp node
 * @param include_mask parameter mask
 * @param exclude_mask parameter mask
 * @param timeout in ms
 * @param version 1 or 2
 * @return 0 = OK, -1 on network error
 */
int param_pull_all(int verbose, int host, uint32_t include_mask, uint32_t exclude_mask, int timeout, int version);

/**
 * PUSH single:
 *
 * Executes an immediate parameter push of a single value.
 *
 * @param param pointer to parameter description
 * @param offset array offset
 * @param value pointer to value (must be aligned type)
 * @param verbose printout when received
 * @param host remote csp node
 * @param timeout in ms
 * @param version 1 or 2
 * @return 0 = OK, -1 on network error
 */
int param_push_single(param_t *param, int offset, void *value, int verbose, int host, int timeout, int version);

/**
 * QUEUE PARAMETER API
 *
 * When dealing with multiple parameters a list can be created using the queues.
 * First create a new queue and add values either getters or setters using the
 * queue functions from param_queue.h
 *
 * Then run PULL on a get queue or PUSH on a set queue. The call is non-destructive
 * so multiple calls to pull/push can be performed using the same queue.
 *
 */


/**
 * PULL/PUSH queue:
 * @param queue pointer to queue
 * @param verbose printout level
 * @param host remote csp node
 * @param timeout in ms
 * @return 0 = OK, -1 on network error
 */
int param_pull_queue(param_queue_t *queue, int verbose, int host, int timeout);
int param_push_queue(param_queue_t *queue, int verbose, int host, int timeout);

#ifdef __cplusplus
}
#endif
#endif /* LIB_PARAM_INCLUDE_PARAM_PARAM_CLIENT_H_ */
