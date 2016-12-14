/*
 * param_client.h
 *
 *  Created on: Dec 14, 2016
 *      Author: johan
 */

#ifndef LIB_PARAM_INCLUDE_PARAM_PARAM_CLIENT_H_
#define LIB_PARAM_INCLUDE_PARAM_PARAM_CLIENT_H_

#include <param/param.h>

int param_push_single(param_t * param, int timeout);
int param_push(param_t * params[], int count, int verbose, int timeout);
int param_pull_single(param_t * param, int timeout);
int param_pull(param_t * params[], int count, int verbose, int timeout);

#endif /* LIB_PARAM_INCLUDE_PARAM_PARAM_CLIENT_H_ */
