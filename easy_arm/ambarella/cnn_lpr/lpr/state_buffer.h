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

#ifndef __STATE_BUFFER_H__
#define __STATE_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CRITICAL_STATE_BUFFER_NUM 3
#define MIN_STATE_BUFFER_NUM 2

typedef struct bbox_param_s bbox_param_t;
typedef struct state_buffer_s {
	uint32_t status;
	uint32_t object_num;
	long timestamp;
	void *img_resource_addr;
	bbox_param_t *bbox_param;
} state_buffer_t;

typedef struct state_buffer_param_s {
	uint16_t verbose;
	uint16_t buffer_num;
	uint16_t max_bbox_num;
	uint16_t img_resource_len;
	state_buffer_t *buf;
} state_buffer_param_t;

int init_state_buffer_param(state_buffer_param_t *state_buffer_param,
	uint16_t buffer_num, uint16_t img_resource_len, uint16_t max_bbox_num, uint16_t verbose);
void deinit_state_buffer_param(state_buffer_param_t *state_buffer_param);
int alloc_single_state_buffer(state_buffer_param_t *state_buffer_param,
	state_buffer_t **buf);
void free_single_state_buffer(state_buffer_t *buf_addr);
int read_state_buffer(state_buffer_t *dst_buffer,
	state_buffer_param_t *state_buffer_param,
	pthread_mutex_t *access_buffer_mutex, sem_t *sem_readable_buf);
int write_state_buffer(state_buffer_param_t *state_buffer_param,
	state_buffer_t *src_buffer, pthread_mutex_t *access_buffer_mutex,
	sem_t *sem_readable_buf, void *covered_img_addr, uint8_t *buffer_covered);

#ifdef __cplusplus
}
#endif

#endif

