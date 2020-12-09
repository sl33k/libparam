/*
 * param_collector.h
 *
 *  Created on: Jan 2, 2017
 *      Author: johan
 */

#ifndef LIB_PARAM_SRC_PARAM_PARAM_COLLECTOR_H_
#define LIB_PARAM_SRC_PARAM_PARAM_COLLECTOR_H_

#include <csp/arch/csp_thread.h>

#ifdef __cplusplus
extern "C" {
#endif

csp_thread_return_t param_collector_task(void *pvParameters);

#ifdef __cplusplus
}
#endif
#endif /* LIB_PARAM_SRC_PARAM_PARAM_COLLECTOR_H_ */
