/*******************************************************************************
 * state_buffer.c
 *
 * History:
 *    2020/03/31  - [Junshuai ZHU] created
 *
 * Copyright (c) 2020 Ambarella International LP
 *
 * This file and its contents ( "Software" ) are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella International LP and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella International LP or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella International LP.
 *
 * This file includes sample code and is only for internal testing and evaluation.  If you
 * distribute this sample code (whether in source, object, or binary code form), it will be
 * without any warranty or indemnity protection from Ambarella International LP or its affiliates.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA INTERNATIONAL LP OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>
#include "ssd_lpr_common.h"
#include "state_buffer.h"

enum buffer_status {
	WRITEABLE,
	READABLE_WRITEABLE,
	STATUS_NUM
};

#define STATE_BUFFER_PRINT_ERROR(rval, str)  \
		printf("\t[Trace] file %s.\n\tfunction: %s: line %d\n", \
		__FILE__, __FUNCTION__, __LINE__); \
		printf("\t%s error! (%d)\n", (str), (rval)); \
		(rval) = -1;

static void print_all_buffer_status(char *period,
	state_buffer_param_t *state_buffer_param)
{
	uint32_t i = 0;
	char *info;

	printf("----------%s buffer status---------------\n", period);
	for (i = 0; i < state_buffer_param->buffer_num; ++i) {
		if (state_buffer_param->buf[i].status == WRITEABLE) {
			info = (char*)"WRITEABLE";
		} else if (state_buffer_param->buf[i].status == READABLE_WRITEABLE){
			info = (char*)"READABLE_WRITEABLE";
		} else {
			info = (char*)"ERROR";
		}
		printf("[%d]: [%s], [%lu], [%p]\n", i, info,
			state_buffer_param->buf[i].timestamp,
			(void*)state_buffer_param->buf[i].img_resource_addr);
	}

	return;
}

static int get_write_idx(state_buffer_param_t *state_buffer_param)
{
	uint16_t i = 0;
	int buffer_idx = -1;
	long timestamp_temp = 0;

	timestamp_temp = (1UL << 63) - 1;
	// find the oldest buffer timestamp
	for (i = 0; i < state_buffer_param->buffer_num; ++i) {
		if (timestamp_temp > state_buffer_param->buf[i].timestamp) {
			timestamp_temp = state_buffer_param->buf[i].timestamp;
			buffer_idx = i;
		}
	}

	return buffer_idx;
}

static int get_read_idx(state_buffer_param_t *state_buffer_param)
{
	uint16_t i = 0;
	int buffer_idx = -1;
	long timestamp_temp = 0;

	if (state_buffer_param->buffer_num <= CRITICAL_STATE_BUFFER_NUM) {
		timestamp_temp = 0;
		// find the newest buffer timestamp
		for (i = 0; i < state_buffer_param->buffer_num; ++i) {
			if (state_buffer_param->buf[i].status == READABLE_WRITEABLE) {
				if (timestamp_temp <= state_buffer_param->buf[i].timestamp) {
					timestamp_temp = state_buffer_param->buf[i].timestamp;
					buffer_idx = i;
				}
			}
		}
	} else {
		timestamp_temp = (1UL << 63) - 1;
		// find the oldest buffer timestamp
		for (i = 0; i < state_buffer_param->buffer_num; ++i) {
			if (state_buffer_param->buf[i].status == READABLE_WRITEABLE) {
				if (timestamp_temp > state_buffer_param->buf[i].timestamp) {
					timestamp_temp = state_buffer_param->buf[i].timestamp;
					buffer_idx = i;
				}
			}
		}
	}

	return buffer_idx;
}

static int check_accessable_buf(state_buffer_param_t *state_buffer_param,
	sem_t *sem_readable_buf)
{
	int rval = 0, sval = 0;
	sem_getvalue(sem_readable_buf, &sval);
	if (sval > state_buffer_param->buffer_num) {
		STATE_BUFFER_PRINT_ERROR(rval, "Accessable buffer num larger than defined num.");
	}

	return rval;
}

int write_state_buffer(state_buffer_param_t *state_buffer_param,
	state_buffer_t *src_buffer, pthread_mutex_t *access_buffer_mutex,
	sem_t *sem_readable_buf, void *covered_img_addr, uint8_t *buffer_covered)
{
	int rval = 0;
	int buffer_idx = 0;
	long timestamp = 0, start_time = 0, end_time = 0;
	struct timeval current_time;

	do {
		if (src_buffer == NULL || state_buffer_param == NULL ||
			access_buffer_mutex == NULL || sem_readable_buf == NULL ||
			covered_img_addr == NULL || buffer_covered == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "write_state_buffer input pointer is NULL.");
			break;
		}
		if (state_buffer_param->verbose) {
			gettimeofday(&current_time, NULL);
			start_time = 1000000 * current_time.tv_sec + current_time.tv_usec;
		}
		pthread_mutex_lock(access_buffer_mutex);
		if (state_buffer_param->verbose) {
			print_all_buffer_status((char*)"[WRITE] ", state_buffer_param);
		}
		buffer_idx = get_write_idx(state_buffer_param);
		if (buffer_idx < 0) {
			STATE_BUFFER_PRINT_ERROR(rval, "No accessable buffer. Stop.");
			break;
		} else {
			gettimeofday(&current_time, NULL);
			timestamp = 1000000 * current_time.tv_sec + current_time.tv_usec;
			state_buffer_param->buf[buffer_idx].timestamp = timestamp;
			if (state_buffer_param->buf[buffer_idx].status == WRITEABLE) {
				sem_post(sem_readable_buf);
				check_accessable_buf(state_buffer_param, sem_readable_buf);
			} else {
				*buffer_covered = 1;
				memcpy(covered_img_addr, state_buffer_param->buf[buffer_idx].img_resource_addr,
					state_buffer_param->img_resource_len);
			}
		}
		state_buffer_param->buf[buffer_idx].object_num = src_buffer->object_num;
		memcpy(state_buffer_param->buf[buffer_idx].img_resource_addr,
			src_buffer->img_resource_addr, state_buffer_param->img_resource_len);
		memcpy(state_buffer_param->buf[buffer_idx].bbox_param, src_buffer->bbox_param,
			sizeof(bbox_param_t) * src_buffer->object_num);
		state_buffer_param->buf[buffer_idx].status = READABLE_WRITEABLE;
		if (state_buffer_param->verbose) {
			print_all_buffer_status((char*)"[WRITE] ", state_buffer_param);
		}
		pthread_mutex_unlock(access_buffer_mutex);
		if (state_buffer_param->verbose) {
			gettimeofday(&current_time, NULL);
			end_time = 1000000 * current_time.tv_sec + current_time.tv_usec;
			printf("Write buf time %ld us\n", end_time - start_time);
		}

	} while (0);

	return rval;
}

static void get_timeout(struct timeval *current_time, uint32_t ms)
{
	current_time->tv_usec += ms * 1000;
	if(current_time->tv_usec >= 1000000) {
		current_time->tv_sec += current_time->tv_usec / 1000000;
		current_time->tv_usec %= 1000000;
	}

	return;
}

int read_state_buffer(state_buffer_t *dst_buffer,
	state_buffer_param_t *state_buffer_param,
	pthread_mutex_t *access_buffer_mutex, sem_t *sem_readable_buf)
{
	int rval = 0;
	int buffer_idx = 0;
	long start_time = 0, end_time = 0;
	struct timeval current_time;
	struct timespec timeout;

	do {
		if (dst_buffer == NULL || state_buffer_param == NULL ||
			access_buffer_mutex == NULL || sem_readable_buf == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "read_state_buffer input pointer is NULL.");
			break;
		}
		check_accessable_buf(state_buffer_param, sem_readable_buf);
		gettimeofday(&current_time, NULL);
		if (state_buffer_param->verbose) {
			start_time = 1000000 * current_time.tv_sec + current_time.tv_usec;
		}
		get_timeout(&current_time, 2000);
		timeout.tv_sec = current_time.tv_sec;
		timeout.tv_nsec = current_time.tv_usec * 1000;
		rval = sem_timedwait(sem_readable_buf, &timeout);
		if (rval < 0) {
			STATE_BUFFER_PRINT_ERROR(rval, "sem_timedwait timeout. Stop.");
			break;
		}
		pthread_mutex_lock(access_buffer_mutex);
		if (state_buffer_param->verbose) {
			print_all_buffer_status((char*)"[READ] ", state_buffer_param);
		}
		buffer_idx = get_read_idx(state_buffer_param);
		if (buffer_idx < 0) {
			STATE_BUFFER_PRINT_ERROR(rval, "No accessable buffer. Stop.");
			break;
		}
		dst_buffer->object_num = state_buffer_param->buf[buffer_idx].object_num;
		memcpy(dst_buffer->img_resource_addr, state_buffer_param->buf[buffer_idx].img_resource_addr,
			state_buffer_param->img_resource_len);
		memcpy(dst_buffer->bbox_param, state_buffer_param->buf[buffer_idx].bbox_param,
			sizeof(bbox_param_t) * state_buffer_param->buf[buffer_idx].object_num);
		state_buffer_param->buf[buffer_idx].status = WRITEABLE;
		if (state_buffer_param->verbose) {
			print_all_buffer_status((char*)"[READ] ", state_buffer_param);
		}
		pthread_mutex_unlock(access_buffer_mutex);
		if (state_buffer_param->verbose) {
			gettimeofday(&current_time, NULL);
			end_time = 1000000 * current_time.tv_sec + current_time.tv_usec;
			printf("Read buf time %ld us\n", end_time - start_time);
		}

	} while (0);

	return rval;
}

void free_single_state_buffer(state_buffer_t *buf_addr)
{
	if (buf_addr != NULL) {
		if (buf_addr->bbox_param != NULL) {
			free(buf_addr->bbox_param);
			buf_addr->bbox_param = NULL;
		}
		if (buf_addr->img_resource_addr!= NULL) {
			free(buf_addr->img_resource_addr);
			buf_addr->img_resource_addr = NULL;
		}
		if (buf_addr != NULL) {
			free(buf_addr);
			buf_addr = NULL;
		}
	}

	return;
}

int alloc_single_state_buffer(state_buffer_param_t *state_buffer_param,
	state_buffer_t **buf)
{
	int rval = 0;
	state_buffer_t *buf_addr = *buf;

	do {
		if (state_buffer_param == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "state_buffer_param_t pointer should not be NULL.");
			break;
		}

		buf_addr = (state_buffer_t*)malloc(sizeof(state_buffer_t));
		if (buf_addr == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "state_buffer_param malloc");
			break;
		}
		memset(buf_addr, 0, sizeof(state_buffer_t));
		*buf = buf_addr;
		buf_addr->bbox_param = (bbox_param_t *)malloc(sizeof(bbox_param_t) *
			state_buffer_param->max_bbox_num);
		if (buf_addr->bbox_param == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "alloc_single_state_buffer bbox malloc");
			free_single_state_buffer(buf_addr);
			break;
		}
		memset(buf_addr->bbox_param, 0, sizeof(bbox_param_t) *
			state_buffer_param->max_bbox_num);
		buf_addr->img_resource_addr = malloc(state_buffer_param->img_resource_len);
		if (buf_addr->img_resource_addr == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "alloc_single_state_buffer malloc img_resource");
			free_single_state_buffer(buf_addr);
			break;
		}
		memset(buf_addr->img_resource_addr, 0, state_buffer_param->img_resource_len);
	} while (0);

	return rval;
}

void deinit_state_buffer_param(state_buffer_param_t *state_buffer_param)
{
	if (state_buffer_param != NULL) {
		if (state_buffer_param->buf != NULL) {
			if (state_buffer_param->buf->bbox_param != NULL) {
				free(state_buffer_param->buf->bbox_param);
				state_buffer_param->buf->bbox_param = NULL;
			}
			if (state_buffer_param->buf->img_resource_addr != NULL) {
				free(state_buffer_param->buf->img_resource_addr);
				state_buffer_param->buf->img_resource_addr = NULL;
			}
			if (state_buffer_param->buf != NULL) {
				free(state_buffer_param->buf);
				state_buffer_param->buf = NULL;
			}
		}
	}

	return;
}

int init_state_buffer_param(state_buffer_param_t *state_buffer_param,
	uint16_t buffer_num, uint16_t img_resource_len, uint16_t max_bbox_num, uint16_t verbose)
{
	int rval = 0;
	uint32_t i = 0;

	do {
		if (state_buffer_param == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "state_buffer_param_t pointer should not be NULL.");
			break;
		}
		if (buffer_num < MIN_STATE_BUFFER_NUM) {
			STATE_BUFFER_PRINT_ERROR(rval, "state_buffer_num should not be smaller than 2.");
			break;
		}
		state_buffer_param->buffer_num = buffer_num;
		state_buffer_param->verbose = verbose;
		state_buffer_param->max_bbox_num = max_bbox_num;
		state_buffer_param->buf =
			(state_buffer_t*)malloc(sizeof(state_buffer_t) * buffer_num);
		if (state_buffer_param->buf == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "state_buffer_t malloc buf");
			break;
		}
		memset(state_buffer_param->buf, 0, sizeof(state_buffer_t) * buffer_num);

		state_buffer_param->buf->bbox_param = (bbox_param_t *)
			malloc(sizeof(bbox_param_t) * max_bbox_num * buffer_num);
		if (state_buffer_param->buf->bbox_param == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "bbox_params malloc");
			deinit_state_buffer_param(state_buffer_param);
			break;
		}
		memset(state_buffer_param->buf->bbox_param, 0,
			sizeof(bbox_param_t) * max_bbox_num * buffer_num);
		for (i = 1; i < buffer_num; ++i) {
			state_buffer_param->buf[i].bbox_param =
				(state_buffer_param->buf->bbox_param + i * max_bbox_num);
		}
		state_buffer_param->img_resource_len = img_resource_len;
		state_buffer_param->buf->img_resource_addr =
			malloc(img_resource_len * buffer_num);
		if (state_buffer_param->buf->img_resource_addr == NULL) {
			STATE_BUFFER_PRINT_ERROR(rval, "state_buffer_t malloc img_resource");
			deinit_state_buffer_param(state_buffer_param);
			break;
		}
		memset(state_buffer_param->buf->img_resource_addr, 0,
			img_resource_len * buffer_num);
		for (i = 1; i < buffer_num; ++i) {
			state_buffer_param->buf[i].img_resource_addr =
				(void*)((uint64_t)state_buffer_param->buf->img_resource_addr +
				i * img_resource_len);
		}
	} while (0);

	return rval;
}

