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

#ifndef __SSD_LPR_COMMON_H__
#define __SSD_LPR_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TIME_MEASURE_START(flag) \
	if ((flag) >= TIME_LEVEL) { \
		debug_time = gettimeus(); \
	}

#define TIME_MEASURE_END(str, flag) \
	if ((flag) >= TIME_LEVEL) { \
		EA_LOG_NOTICE("%s: %lu ms (%lu us)\n", (str), \
		(gettimeus() - (debug_time)) / 1000, (gettimeus() - (debug_time))); \
	}

#define SAVE_TENSOR_IN_DEBUG_MODE(filename, tensor, flag) \
	if ((flag) == DEBUG_LEVEL) { \
		RVAL_OK(ea_tensor_to_jpeg(tensor, EA_TENSOR_COLOR_MODE_BGR, (filename))); \
	}

#define SAVE_TENSOR_GROUP_IN_DEBUG_MODE(file_prefix, idx, tensor, flag) \
		if ((flag) == DEBUG_LEVEL) { \
			std::stringstream filename; \
			filename << (file_prefix) << (idx) << ".jpg"; \
			RVAL_OK(ea_tensor_to_jpeg(tensor, EA_TENSOR_COLOR_MODE_BGR, (filename).str().c_str())); \
		}

#define SAVE_MAT_GROUP_IN_DEBUG_MODE(file_prefix, idx, img, flag) \
		if ((flag) == DEBUG_LEVEL) { \
			save_mat_to_file((img), (file_prefix), (idx)); \
		}

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

enum debug_config {
	TIME_LEVEL = 1,
	INFO_LEVEL = 2,
	DEBUG_LEVEL = 3,
	MAX_DEBUG_NUM
};

typedef struct bbox_param_s {
	float norm_min_x;
	float norm_min_y;
	float norm_max_x;
	float norm_max_y;
} bbox_param_t;

#ifdef __cplusplus
}
#endif

#endif

