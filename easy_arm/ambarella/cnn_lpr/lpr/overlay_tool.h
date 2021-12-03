/*******************************************************************************
 * overlay_tool.h
 *
 * History:
 *  2020/08/24  - [Junshuai ZHU] created
 *
 *
 * Copyright (C) 2020 Ambarella International LP
 *
 * This file and its contents ("Software") are protected by intellectual
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

#ifndef __OVERLAY_TOOL_H__
#define __OVERLAY_TOOL_H__

#ifndef STRING_SIZE
#define STRING_SIZE				(256)
#endif

#define MAX_OVERLAY_PLATE_NUM		(30)
#define DEFAULT_OVERLAY_LICENSE_NUM	(8)
#define MIN_OVERLAY_PLATE_NUM		(0)
#define MIN_X_OFFSET			(0.0f)
#define DEFAULT_X_OFFSET		(0.0f)
#define MAX_X_OFFSET			(1.0f)
#define MIN_HIGHLIGHT_SEC		(0)
#define DEFAULT_HIGHLIGHT_SEC	(5)
#define MAX_HIGHLIGHT_SEC		(1 << 15)
#define MIN_CLEAR_SEC			(0)
#define DEFAULT_CLEAR_SEC		(30)
#define MAX_CLEAR_SEC			(1 << 15)
#define MIN_WIDTH_RATIO			(0.9f)
#define DEFAULT_WIDTH_RATIO		(1.0f)
#define MAX_WIDTH_RATIO			(1.0f)
#define MIN_STREAM_ID			(0)
#define MAX_STREAM_ID			(13 - 1)
#define DRAW_PLATE_HEIGHT		(64)
#define DRAW_PLATE_WIDTH		(256)

typedef struct bbox_param_s bbox_param_t;

typedef struct bbox_list_s {
	bbox_param_t bbox[MAX_OVERLAY_PLATE_NUM];
	uint32_t bbox_num;
} bbox_list_t;

typedef struct license_plate_s {
	char text[STRING_SIZE];
	float conf;
	uint32_t reserve;
	bbox_param_t bbox;
	ea_tensor_t *img_tensor;
	uint64_t timestamp;
} license_plate_t;

typedef struct draw_plate_list_s {
	license_plate_t license_plate[MAX_OVERLAY_PLATE_NUM];
	uint32_t license_num;
} draw_plate_list_t;

int set_overlay_bbox(bbox_list_t *bbox_list);
int set_overlay_image(ea_tensor_t *complete_img, draw_plate_list_t* draw_plate_list);
int show_overlay(uint32_t dsp_pts);
int init_overlay_tool(int stream_id, float x_offset,
	uint32_t highlight_sec, uint32_t clear_sec, float width_ratio,
	uint32_t draw_plate_num, uint32_t debug_en);
void deinit_overlay_tool();


#endif

