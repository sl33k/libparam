/*
 * vmem_client_slash.c
 *
 *  Created on: Oct 27, 2016
 *      Author: johan
 */


#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <csp/csp.h>
#include <csp/arch/csp_time.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <vmem/vmem_client.h>
#include <vmem/vmem_server.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>

#include <slash/slash.h>

static int vmem_client_slash_list(struct slash *slash)
{
	int node = csp_get_address();
	int timeout = 2000;
	char * endptr;

	if (slash->argc >= 2) {
		node = strtoul(slash->argv[1], &endptr, 10);
		if (*endptr != '\0')
			return SLASH_EUSAGE;
	}

	if (slash->argc >= 3) {
		timeout = strtoul(slash->argv[2], &endptr, 10);
		if (*endptr != '\0')
			return SLASH_EUSAGE;
	}

	printf("Requesting vmem list from node %u timeout %u\n", node, timeout);

	vmem_client_list(node, timeout);
	return SLASH_SUCCESS;
}
slash_command_sub(vmem, list, vmem_client_slash_list, "[node] [timeout]", NULL);

static int vmem_client_slash_fram(struct slash *slash, int backup) {

	int node = csp_get_address();
	int vmem_id;
	int timeout = 2000;
	char * endptr;

	if (slash->argc < 2)
		return SLASH_EUSAGE;

	vmem_id = strtoul(slash->argv[1], &endptr, 10);
	if (*endptr != '\0')
		return SLASH_EUSAGE;

	if (slash->argc >= 3) {
		node = strtoul(slash->argv[2], &endptr, 10);
		if (*endptr != '\0')
			return SLASH_EUSAGE;
	}

	if (slash->argc >= 4) {
		timeout = strtoul(slash->argv[3], &endptr, 10);
		if (*endptr != '\0')
			return SLASH_EUSAGE;
	}

	if (backup) {
		printf("Taking backup of vmem %u on node %u\n", vmem_id, node);
	} else {
		printf("Restoring vmem %u on node %u\n", vmem_id, node);
	}

	int result = vmem_client_backup(node, vmem_id, timeout, backup);
	if (result == -2) {
		printf("No response\n");
	} else {
		printf("Result: %d\n", result);
	}

	return SLASH_SUCCESS;
}

static int vmem_client_slash_restore(struct slash *slash)
{
	return vmem_client_slash_fram(slash, 0);
}
slash_command_sub(vmem, restore, vmem_client_slash_restore, "<vmem idx> [node] [timeout]", NULL);

static int vmem_client_slash_backup(struct slash *slash)
{
	return vmem_client_slash_fram(slash, 1);
}
slash_command_sub(vmem, backup, vmem_client_slash_backup, "<vmem idx> [node] [timeout]", NULL);

static int vmem_client_slash_unlock(struct slash *slash)
{
	int node = csp_get_address();
	int timeout = 2000;
	char * endptr;

	if (slash->argc >= 2) {
		node = strtoul(slash->argv[1], &endptr, 10);
		if (*endptr != '\0')
			return SLASH_EUSAGE;
	}

	if (slash->argc >= 3) {
		timeout = strtoul(slash->argv[2], &endptr, 10);
		if (*endptr != '\0')
			return SLASH_EUSAGE;
	}

	/* Step 0: Prepare request */
	csp_conn_t * conn = csp_connect(CSP_PRIO_HIGH, node, VMEM_PORT_SERVER, timeout, CSP_O_NONE);
	if (conn == NULL)
		return SLASH_EINVAL;

	csp_packet_t * packet = csp_buffer_get(sizeof(vmem_request_t));
	if (packet == NULL)
		return SLASH_EINVAL;

	vmem_request_t * request = (void *) packet->data;
	request->version = 1;
	request->type = VMEM_SERVER_UNLOCK;
	packet->length = sizeof(vmem_request_t);

	/* Step 1: Check initial unlock code */
	request->unlock.code = csp_hton32(0x28140360);

	if (!csp_send(conn, packet, 0)) {
		csp_buffer_free(packet);
		csp_close(conn);
		return SLASH_EINVAL;
	}

	/* Step 2: Wait for verification sequence */
	if ((packet = csp_read(conn, timeout)) == NULL) {
		csp_close(conn);
		return SLASH_EINVAL;
	}

	request = (void *) packet->data;
	uint32_t sat_verification = csp_ntoh32(request->unlock.code);

	printf("Verification code received: %x\n\n", (unsigned int) sat_verification);

	printf("************************************\n");
	printf("* WARNING WARNING WARNING WARNING! *\n");
	printf("* You are about to unlock the FRAM *\n");
	printf("* Please understand the risks      *\n");
	printf("* Abort now by typing CTRL + C     *\n");
	printf("************************************\n");

	/* Step 2a: Ask user to input sequence */
	uint32_t user_verification;
	printf("Type verification sequence (you have <30 seconds): \n");

	char readbuf[9] = {};
	int cnt = 0;
	while(cnt < 8) {
		cnt += read(0, readbuf + cnt, 8-cnt);
	}
	if (sscanf(readbuf, "%x", (unsigned int *) &user_verification) != 1) {
		printf("Could not parse input\n");
		return SLASH_EINVAL;
	}

	printf("User input: %x\n", (unsigned int) user_verification);
	if (user_verification != sat_verification) {
		csp_buffer_free(packet);
		csp_close(conn);
		return SLASH_EINVAL;
	}

	/* Step 2b: Ask for final confirmation */
	printf("Validation sequence accepted\n");

	printf("Are you sure [Y/N]?\n");

	cnt = read(0, readbuf, 1);
	if (readbuf[0] != 'Y') {
		csp_buffer_free(packet);
		csp_close(conn);
		return SLASH_EINVAL;
	}

	/* Step 3: Send verification sequence */
	request->unlock.code = csp_hton32(user_verification);

	if (!csp_send(conn, packet, 0)) {
		csp_buffer_free(packet);
		csp_close(conn);
		return SLASH_EINVAL;
	}

	/* Step 4: Check for result */
	if ((packet = csp_read(conn, timeout)) == NULL) {
		csp_close(conn);
		return SLASH_EINVAL;
	}

	request = (void *) packet->data;
	uint32_t result = csp_ntoh32(request->unlock.code);
	printf("Result: %x\n", (unsigned int) result);

	csp_close(conn);
	return SLASH_SUCCESS;

}
slash_command_sub(vmem, unlock, vmem_client_slash_unlock, "[node] [timeout]", NULL);
