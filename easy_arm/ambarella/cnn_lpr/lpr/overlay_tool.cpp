/*******************************************************************************
 * overlay_tool.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sstream>
#include <basetypes.h>
#include <unistd.h>
#include <eazyai.h>
#include "ssd_lpr_common.h"
#include "overlay_tool.h"

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);

typedef struct overlay_ctx_s {
	int is_init;
	int run_flag;
	int stream_id;
	uint32_t dsp_pts;
	uint32_t debug_en;
	uint32_t highlight_sec;
	uint32_t clear_sec;
	uint32_t draw_plate_num;
	float display_w;
	float display_h;
	float x_offset;
	float width_ratio;
	int update_cond_flag;
	pthread_cond_t update_cond;
	pthread_mutex_t bbox_mutex;
	pthread_mutex_t plate_mutex;
	pthread_t overlay_pthread_id;
	bbox_list_t bbox_list;
	draw_plate_list_t draw_plate_list;
	draw_plate_list_t final_plate_list;
	ea_display_t *display;
} overlay_ctx_t;

static overlay_ctx_t overlay_ctx;

static unsigned long gettimeus(void)
{
	static struct timeval tv;

	gettimeofday(&tv, NULL);

	return (unsigned long) 1000000 * (unsigned long) (tv.tv_sec) + (unsigned long) (tv.tv_usec);
}

static int plate_exchange(license_plate_t *dst_plate, license_plate_t *src_plate)
{
	int rval = 0;
	ea_tensor_t *middle_tensor_addr = NULL;

	do {
		RVAL_ASSERT(dst_plate != NULL);
		RVAL_ASSERT(src_plate != NULL);
		dst_plate->conf = src_plate->conf;
		dst_plate->timestamp = src_plate->timestamp;
		middle_tensor_addr = dst_plate->img_tensor;
		dst_plate->img_tensor = src_plate->img_tensor;
		src_plate->img_tensor = middle_tensor_addr;
		dst_plate->bbox = src_plate->bbox;
		RVAL_ASSERT(sizeof(src_plate->text) <= sizeof(dst_plate->text));
		memset(dst_plate->text, 0, sizeof(dst_plate->text));
		strcpy(dst_plate->text, src_plate->text);
		dst_plate->text[sizeof(dst_plate->text) - 1] = '\0';
	} while (0);

	return rval;
}

static int remove_same_plate_by_text(pthread_mutex_t *mutex,
	draw_plate_list_t *old_plate_list, draw_plate_list_t *new_plate_list)
{
	int rval = 0, i = 0;
	uint32_t j;
	uint32_t idx = 0;
	uint32_t shift_flag[MAX_OVERLAY_PLATE_NUM];
	int new_list_num = min(new_plate_list->license_num, MAX_OVERLAY_PLATE_NUM);

	do {
		RVAL_ASSERT(old_plate_list != NULL);
		RVAL_ASSERT(new_plate_list != NULL);
		RVAL_ASSERT(pthread_mutex_trylock(mutex));
		memset(shift_flag, 0, sizeof(int) * MAX_OVERLAY_PLATE_NUM);
		for (i = 0; i < new_list_num; ++i) {
			for (j = 0; j < old_plate_list->license_num; ++j) {
				if (!strcmp(new_plate_list->license_plate[i].text,
					old_plate_list->license_plate[j].text)) {
					shift_flag[i] = 1;
					old_plate_list->license_plate[j].timestamp = gettimeus();
				}
			}
		}
		for (i = 0; i < new_list_num; ++i) {
			if (shift_flag[i] == 0) {
				new_plate_list->license_plate[idx] = new_plate_list->license_plate[i];
				++idx;
			}
		}
		new_plate_list->license_num = idx;
	} while (0);

	return rval;
}

static int sort_list_by_timestamp(draw_plate_list_t *plate_list)
{
	int rval = 0;
	uint32_t i = 0, j = 0;
	license_plate_t middle_license_plate;

	do {
		RVAL_ASSERT(plate_list != NULL);
		memset(&middle_license_plate, 0 , sizeof(middle_license_plate));
		for (i = 0; i < plate_list->license_num; ++i) {
			for (j = i + 1; j < plate_list->license_num; ++j) {
				if (plate_list->license_plate[i].timestamp < plate_list->license_plate[j].timestamp) {
					RVAL_OK(plate_exchange(&middle_license_plate, &plate_list->license_plate[i]));
					RVAL_OK(plate_exchange(&plate_list->license_plate[i], &plate_list->license_plate[j]));
					RVAL_OK(plate_exchange(&plate_list->license_plate[j], &middle_license_plate));
				}
			}
			if (rval < 0) {
				break;
			}
		}
	} while (0);

	return rval;
}

int set_overlay_image(ea_tensor_t *complete_img, draw_plate_list_t* draw_plate_list)
{
	uint32_t i = 0;
	int rval = 0;
	int j = 0;
	int old_list_num = 0, add_list_num = 0, abandon_num = 0, left_over_num = 0, last_index = 0;
	ea_tensor_t *complete_img_y = NULL;
	ea_tensor_t *cropped_y_img[MAX_OVERLAY_PLATE_NUM];
	ea_roi_t roi_group[MAX_OVERLAY_PLATE_NUM];
	uint64_t debug_time = 0;
	uint64_t start_time = gettimeus();

	do {
		RVAL_ASSERT(complete_img != NULL);
		RVAL_ASSERT(draw_plate_list != NULL);
		memset(cropped_y_img, 0, sizeof(uint32_t) * MAX_OVERLAY_PLATE_NUM);
		memset(roi_group, 0, sizeof(ea_roi_t) * MAX_OVERLAY_PLATE_NUM);
		if (draw_plate_list->license_num == 0) {
			break;
		}
		complete_img_y = ea_tensor_new_from_other(complete_img, 0);
		pthread_mutex_lock(&overlay_ctx.plate_mutex);
		if (overlay_ctx.debug_en >= TIME_LEVEL) {
			EA_LOG_NOTICE("Overlay wait for plate lock time: %lu ms AKA %lu us\n",
				(gettimeus() - start_time) / 1000, (gettimeus() - start_time));
		}

		RVAL_OK(remove_same_plate_by_text(&overlay_ctx.plate_mutex,
			&overlay_ctx.draw_plate_list, draw_plate_list));
		if (draw_plate_list->license_num == 0) {
			ea_tensor_free(complete_img_y);
			pthread_mutex_unlock(&overlay_ctx.plate_mutex);
			break;
		}
		old_list_num = overlay_ctx.draw_plate_list.license_num;
		add_list_num = min(draw_plate_list->license_num, MAX_OVERLAY_PLATE_NUM);
		abandon_num = max(old_list_num + add_list_num - MAX_OVERLAY_PLATE_NUM, 0);
		left_over_num = old_list_num - abandon_num; // abandon_num always <= old_list_num, since add_list_num <= MAX_OVERLAY_PLATE_NUM
		last_index = min(old_list_num + add_list_num - 1, MAX_OVERLAY_PLATE_NUM - 1); // add_list_num always >= 1
		RVAL_ASSERT(old_list_num <= MAX_OVERLAY_PLATE_NUM && add_list_num <= MAX_OVERLAY_PLATE_NUM
			&& abandon_num <= MAX_OVERLAY_PLATE_NUM && left_over_num <= MAX_OVERLAY_PLATE_NUM
			&& last_index <= MAX_OVERLAY_PLATE_NUM);

		// shift the old list down
		for (j = 0; j < left_over_num; ++j) {
			RVAL_OK(plate_exchange(&(overlay_ctx.draw_plate_list.license_plate[last_index - j]),
				&(overlay_ctx.draw_plate_list.license_plate[left_over_num - j - 1])));
		}
		if (rval < 0) {
			break;
		}

		// add new list
		for (i = 0; i < (uint32_t)add_list_num; ++i) {
			cropped_y_img[i] = overlay_ctx.draw_plate_list.license_plate[i].img_tensor;
			roi_group[i].x = draw_plate_list->license_plate[i].bbox.norm_min_x * ea_tensor_shape(complete_img_y)[3];
			roi_group[i].y = draw_plate_list->license_plate[i].bbox.norm_min_y * ea_tensor_shape(complete_img_y)[2];
			roi_group[i].w = (draw_plate_list->license_plate[i].bbox.norm_max_x -
				draw_plate_list->license_plate[i].bbox.norm_min_x) * ea_tensor_shape(complete_img_y)[3];
			roi_group[i].h = (draw_plate_list->license_plate[i].bbox.norm_max_y -
				draw_plate_list->license_plate[i].bbox.norm_min_y) * ea_tensor_shape(complete_img_y)[2];
			overlay_ctx.draw_plate_list.license_plate[i].bbox =
				draw_plate_list->license_plate[i].bbox;
			overlay_ctx.draw_plate_list.license_plate[i].conf =
				draw_plate_list->license_plate[i].conf;
			memset(overlay_ctx.draw_plate_list.license_plate[i].text, 0,
				sizeof(overlay_ctx.draw_plate_list.license_plate[i].text));
			strncpy(overlay_ctx.draw_plate_list.license_plate[i].text,
				draw_plate_list->license_plate[i].text,
				sizeof(overlay_ctx.draw_plate_list.license_plate[i].text) - 1);
			overlay_ctx.draw_plate_list.license_plate[i].text[
				sizeof(overlay_ctx.draw_plate_list.license_plate[i].text) - 1] = '\0';
			overlay_ctx.draw_plate_list.license_plate[i].timestamp = gettimeus();
		}
		overlay_ctx.draw_plate_list.license_num = min(old_list_num + add_list_num,
			MAX_OVERLAY_PLATE_NUM);
		TIME_MEASURE_START(overlay_ctx.debug_en);
		RVAL_OK(ea_crop_resize(&complete_img_y, 1, cropped_y_img, add_list_num,
			roi_group, EA_TENSOR_COLOR_MODE_GRAY, EA_VP));
		TIME_MEASURE_END("Overlay ea_crop_resize time", overlay_ctx.debug_en);
		pthread_mutex_unlock(&overlay_ctx.plate_mutex);
		ea_tensor_free(complete_img_y);
	} while (0);
	if (rval < 0) {
		pthread_mutex_unlock(&overlay_ctx.plate_mutex);
		ea_tensor_free(complete_img_y);
	}
	if (overlay_ctx.debug_en >= TIME_LEVEL) {
		EA_LOG_NOTICE("Overlay set image complete time: %lu ms AKA %lu us\n",
			(gettimeus() - start_time) / 1000, (gettimeus() - start_time));
	}

	return rval;
}

static void * show_overlay_thread(void* overlay_thread_param)
{
	int rval = 0;
	uint32_t i = 0;
	float plate_w = DRAW_PLATE_WIDTH / overlay_ctx.display_w;
	float plate_h = DRAW_PLATE_HEIGHT / overlay_ctx.display_h;
	float plate_y = 0.05f; // Leave some space for the start position
	uint32_t draw_plate_num = 0;
	uint64_t diff_sec = 0;
	struct timespec outtime;
	struct timeval now;

	while (overlay_ctx.run_flag) {
		pthread_mutex_lock(&overlay_ctx.plate_mutex);
		RVAL_OK(sort_list_by_timestamp(&overlay_ctx.draw_plate_list));
		draw_plate_num = min(overlay_ctx.draw_plate_list.license_num, overlay_ctx.draw_plate_num);
		for (i = 0; i < draw_plate_num; ++i) {
			RVAL_ASSERT(sizeof(overlay_ctx.draw_plate_list.license_plate[i].text) <=
				sizeof(overlay_ctx.final_plate_list.license_plate[i].text));
			memset(overlay_ctx.final_plate_list.license_plate[i].text, 0,
				sizeof(overlay_ctx.final_plate_list.license_plate[i].text));
			strcpy(overlay_ctx.final_plate_list.license_plate[i].text,
				overlay_ctx.draw_plate_list.license_plate[i].text);
			overlay_ctx.final_plate_list.license_plate[i].text[sizeof(
				overlay_ctx.final_plate_list.license_plate[i].text) - 1] = '\0';
			RVAL_ASSERT(ea_tensor_size(overlay_ctx.draw_plate_list.license_plate[i].img_tensor) <=
				ea_tensor_size(overlay_ctx.final_plate_list.license_plate[i].img_tensor));
			memcpy(ea_tensor_data(overlay_ctx.final_plate_list.license_plate[i].img_tensor),
				ea_tensor_data(overlay_ctx.draw_plate_list.license_plate[i].img_tensor),
				ea_tensor_size(overlay_ctx.draw_plate_list.license_plate[i].img_tensor));
			overlay_ctx.final_plate_list.license_plate[i].timestamp =
				overlay_ctx.draw_plate_list.license_plate[i].timestamp;
		}
		pthread_mutex_unlock(&overlay_ctx.plate_mutex);

		ea_display_obj_params(overlay_ctx.display)->title_pos = EA_TITLE_POS_RIGHT;
		ea_display_obj_params(overlay_ctx.display)->text_background_color = EA_16_COLORS_GRAY;
		ea_display_obj_params(overlay_ctx.display)->text_background_transparency = 128;
		ea_display_obj_params(overlay_ctx.display)->font_size = 44;
		plate_y = 0.05f;
		for (i = 0; i < draw_plate_num; ++i) {
			diff_sec = (gettimeus() -
				overlay_ctx.final_plate_list.license_plate[i].timestamp) / 1000000;
			if (diff_sec > overlay_ctx.clear_sec) {
				continue;
			} else if (diff_sec > overlay_ctx.highlight_sec) {
				ea_display_obj_params(overlay_ctx.display)->text_color = EA_16_COLORS_BLACK;
			} else {
				ea_display_obj_params(overlay_ctx.display)->text_color = EA_16_COLORS_RED;
			}
			SAVE_TENSOR_GROUP_IN_DEBUG_MODE("draw_overlay", i,
				overlay_ctx.final_plate_list.license_plate[i].img_tensor, overlay_ctx.debug_en)
			RVAL_OK(ea_display_set_gray_image(overlay_ctx.display,
				overlay_ctx.final_plate_list.license_plate[i].text,
				overlay_ctx.x_offset, plate_y, plate_w, plate_h,
				overlay_ctx.final_plate_list.license_plate[i].img_tensor));
			plate_y += plate_h;
			if (plate_y > 1 - plate_h) {
				break;
			}
		}
		if (rval < 0) {
			break;
		}

		gettimeofday(&now, NULL);
		outtime.tv_sec = now.tv_sec + 1; // wait at most 1 sec or quit
		outtime.tv_nsec = now.tv_usec * 1000;
		pthread_mutex_lock(&overlay_ctx.bbox_mutex);
		while (overlay_ctx.update_cond_flag == 0) {
			rval = pthread_cond_timedwait(&overlay_ctx.update_cond,
				&overlay_ctx.bbox_mutex, &outtime);
			if (rval == ETIMEDOUT) {
				printf("%s, line %d: wait time out, quit.\n", __FUNCTION__, __LINE__);
				overlay_ctx.run_flag = 0;
				rval = -1;
				break;
			}
		}
		if (overlay_ctx.run_flag == 0) {
			break;
		}
		overlay_ctx.update_cond_flag = 0;
		for (i = 0; i < overlay_ctx.bbox_list.bbox_num; ++i) {
			RVAL_OK(ea_display_set_bbox(overlay_ctx.display, "",
				overlay_ctx.bbox_list.bbox[i].norm_min_x, overlay_ctx.bbox_list.bbox[i].norm_min_y,
				overlay_ctx.bbox_list.bbox[i].norm_max_x - overlay_ctx.bbox_list.bbox[i].norm_min_x,
				overlay_ctx.bbox_list.bbox[i].norm_max_y - overlay_ctx.bbox_list.bbox[i].norm_min_y));
		}
		RVAL_OK(ea_display_refresh(overlay_ctx.display, (void *)(uint64_t)overlay_ctx.dsp_pts));
		pthread_mutex_unlock(&overlay_ctx.bbox_mutex);
	}
	if (rval < 0) {
		pthread_mutex_unlock(&overlay_ctx.bbox_mutex);
	}
	EA_LOG_NOTICE("show_overlay_thread quit.\n");

	return NULL;
}

int show_overlay(uint32_t dsp_pts)
{
	int rval = 0;

	do {
		pthread_mutex_lock(&overlay_ctx.bbox_mutex);
		overlay_ctx.dsp_pts = dsp_pts;
		overlay_ctx.update_cond_flag = 1;
		pthread_cond_signal(&overlay_ctx.update_cond);
		pthread_mutex_unlock(&overlay_ctx.bbox_mutex);
	} while (0);

	return rval;
}


int set_overlay_bbox(bbox_list_t *bbox_list)
{
	int rval = 0;
	uint32_t i = 0;

	do {
		RVAL_ASSERT(bbox_list != NULL);
		pthread_mutex_lock(&(overlay_ctx.bbox_mutex));
		overlay_ctx.bbox_list.bbox_num = bbox_list->bbox_num;
		for (i = 0; i < bbox_list->bbox_num; ++i) {
			overlay_ctx.bbox_list.bbox[i] = bbox_list->bbox[i];
		}
		pthread_mutex_unlock(&overlay_ctx.bbox_mutex);
	} while (0);

	return rval;
}

static int check_param()
{
	int rval = 0;

	do {
		RVAL_ASSERT(MAX_OVERLAY_PLATE_NUM >= 1);
		RVAL_ASSERT(overlay_ctx.x_offset < MAX_X_OFFSET || overlay_ctx.x_offset > MIN_X_OFFSET);
		RVAL_ASSERT(overlay_ctx.width_ratio < MAX_WIDTH_RATIO || overlay_ctx.width_ratio > MIN_WIDTH_RATIO);
		RVAL_ASSERT(overlay_ctx.highlight_sec < MAX_HIGHLIGHT_SEC || overlay_ctx.highlight_sec > MIN_HIGHLIGHT_SEC);
		RVAL_ASSERT(overlay_ctx.clear_sec < MAX_CLEAR_SEC || overlay_ctx.clear_sec > MIN_CLEAR_SEC);
		RVAL_ASSERT(overlay_ctx.stream_id < MAX_STREAM_ID || overlay_ctx.stream_id > MIN_STREAM_ID);
		RVAL_ASSERT(overlay_ctx.draw_plate_num < MAX_OVERLAY_PLATE_NUM || overlay_ctx.draw_plate_num > MIN_OVERLAY_PLATE_NUM);
		RVAL_ASSERT(overlay_ctx.debug_en < MAX_DEBUG_NUM);
	} while (0);

	return rval;
}

int init_overlay_tool(int stream_id, float x_offset,
	uint32_t highlight_sec, uint32_t clear_sec, float width_ratio,
	uint32_t draw_plate_num, uint32_t debug_en)
{
	int i, rval = 0;
	size_t license_img_shape[4];

	do {
		overlay_ctx.x_offset = x_offset;
		overlay_ctx.highlight_sec = highlight_sec;
		overlay_ctx.clear_sec = clear_sec;
		overlay_ctx.width_ratio = width_ratio;
		overlay_ctx.stream_id = stream_id;
		overlay_ctx.draw_plate_num = draw_plate_num;
		overlay_ctx.debug_en = debug_en;
		check_param();
		pthread_mutex_init(&overlay_ctx.bbox_mutex, NULL);
		pthread_mutex_init(&overlay_ctx.plate_mutex, NULL);
		pthread_cond_init(&overlay_ctx.update_cond, NULL);
		overlay_ctx.update_cond_flag = 1;
		overlay_ctx.display = ea_display_new(EA_DISPLAY_STREAM,
			overlay_ctx.stream_id, EA_DISPLAY_BBOX_TEXTBOX + EA_DISPLAY_GRAY_SCALE, NULL);
		RVAL_ASSERT(overlay_ctx.display != NULL);
		ea_display_obj_params(overlay_ctx.display)->font_width_ratio = overlay_ctx.width_ratio;
		ea_display_obj_params(overlay_ctx.display)->border_thickness = 4;
		ea_display_obj_params(overlay_ctx.display)->obj_win_w = 1.0;
		ea_display_obj_params(overlay_ctx.display)->obj_win_h = 1.0;
		ea_display_obj_params(overlay_ctx.display)->box_color = EA_16_COLORS_WHITE;
		ea_display_obj_params(overlay_ctx.display)->locale = LOCALE_CHINESE;
		ea_display_obj_params(overlay_ctx.display)->font_path = EA_DEFAULT_CHINESE_FONT_PATH;
		overlay_ctx.display_w = ea_display_obj_params(overlay_ctx.display)->dis_win_w;
		overlay_ctx.display_h = ea_display_obj_params(overlay_ctx.display)->dis_win_h;
		license_img_shape[0] = 1;
		license_img_shape[1] = 1;
		license_img_shape[2] = DRAW_PLATE_HEIGHT;
		license_img_shape[3] = DRAW_PLATE_WIDTH;
		for (i = 0; i < MAX_OVERLAY_PLATE_NUM; ++i) {
			overlay_ctx.draw_plate_list.license_plate[i].img_tensor =
				ea_tensor_new(EA_U8, license_img_shape, 0);
			overlay_ctx.final_plate_list.license_plate[i].img_tensor =
				ea_tensor_new(EA_U8, license_img_shape, 0);
		}
		overlay_ctx.run_flag = 1;
		RVAL_OK(pthread_create(&overlay_ctx.overlay_pthread_id, NULL,
			show_overlay_thread, (void*)NULL));
		overlay_ctx.is_init = 1;
	} while (0);

	return rval;
}

void deinit_overlay_tool()
{
	int i;

	if (overlay_ctx.is_init) {
		overlay_ctx.run_flag = 0;
		pthread_mutex_lock(&overlay_ctx.bbox_mutex);
		overlay_ctx.update_cond_flag = 1;
		pthread_cond_signal(&overlay_ctx.update_cond);
		pthread_mutex_unlock(&overlay_ctx.bbox_mutex);

		pthread_join(overlay_ctx.overlay_pthread_id, NULL);
		pthread_mutex_destroy(&overlay_ctx.bbox_mutex);
		pthread_mutex_destroy(&overlay_ctx.plate_mutex);
		pthread_cond_destroy(&overlay_ctx.update_cond);
		if (overlay_ctx.display) {
			ea_display_free(overlay_ctx.display);
		}
		overlay_ctx.display = NULL;
		for (i = 0; i < MAX_OVERLAY_PLATE_NUM; ++i) {
			ea_tensor_free(overlay_ctx.draw_plate_list.license_plate[i].img_tensor);
			ea_tensor_free(overlay_ctx.final_plate_list.license_plate[i].img_tensor);
		}
		printf("overlay quit.\n");
	}

	return;
}

