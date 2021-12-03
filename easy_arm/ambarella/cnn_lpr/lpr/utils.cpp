/*******************************************************************************
 * utils.cpp
 *
 * History:
 *    2020/09/01  - [Junshuai ZHU] created
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
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>
#include <opencv2/opencv.hpp>
#include "utils.hpp"

void convert_rgb_to_mat(cv::Mat &img, unsigned char *bin, int is_bgr,
	int padding)
{
	unsigned int height = img.rows;
	unsigned int width = img.cols;
	unsigned int pitch = padding? (((img.cols) + 31) >> 5) << 5 : img.cols;
	unsigned char *p_d_r, *p_d_g, *p_d_b;
	unsigned int i,j;
	unsigned int h_i = 0;
	unsigned char *p_s = NULL;

	if (is_bgr) {
		p_d_b = bin;
		p_d_g = bin + height * pitch;
		p_d_r = bin + height * pitch * 2;
	} else {
		p_d_r = bin;
		p_d_g = bin + height * pitch;
		p_d_b = bin + height * pitch * 2;
	}
	for (i = 0; i < height; ++i) {
		p_s = img.data + i * width * 3;
		for (j = 0; j < width; ++j) {
			h_i = j * 3;
			p_s[h_i] = p_d_b[0];
			p_s[h_i + 1] = p_d_g[0];
			p_s[h_i + 2] = p_d_r[0];
			p_d_r++;
			p_d_g++;
			p_d_b++;
		}
		p_d_r += pitch - width;
		p_d_g += pitch - width;
		p_d_b += pitch - width;
	}

	return;
}

cv::Mat convert_yuv_to_bgr24(uint8_t *nv12_buffer, uint8_t *y_addr,
	uint8_t *uv_addr, int width, int height, int pitch)
{
	int i;
	uint8_t *in = NULL, *out = NULL;
	cv::Mat frame(height, width, CV_8UC3);
	cv::Mat src((height + height / 2), width, CV_8UC1, nv12_buffer);

	// Copy to malloc buffer and Remove Pitch
	in = y_addr;
	out = nv12_buffer;
	for (i = 0; i < height; i++) {
		memcpy(out, in, width);
		in += pitch;
		out += width;
	}

	in = uv_addr;
	out = nv12_buffer + width * height;
	for (i = 0; i < height / 2; i++) {
		memcpy(out, in, width);
		in += pitch;
		out += width;
	}

	// Convert NV12 to BGR
#if CV_VERSION_MAJOR < 4
	cv::cvtColor(src, frame, CV_YUV2BGR_NV12);
#else
	cv::cvtColor(src, frame, COLOR_YUV2BGR_NV12);
#endif

	return frame;
}

int convert_cvMat_to_rgb(cv::Mat cv_mat, uint8_t *rgb, int is_bgr,
	int padding)
{
	uint32_t height = cv_mat.rows;
	uint32_t width = cv_mat.cols;
	uint32_t pitch = padding? (((cv_mat.cols) + 31) >> 5) << 5 : cv_mat.cols;
	int rval = 0;
	uint32_t i, j;
	uint8_t *p_d_r = rgb, *p_d_g,*p_d_b;
	int h_i = 0;
	uint8_t *p_s = NULL;

	if (is_bgr) {
		p_d_b = rgb;
		p_d_g = p_d_b + height * pitch;
		p_d_r = p_d_g + height * pitch;
	} else {
		p_d_r = rgb;
		p_d_g = p_d_r + height * pitch;
		p_d_b = p_d_g + height * pitch;
	}

	do {
		if (!cv_mat.data) {
			printf("error: cv_mat is NULL.\n");
			rval = -1;
			break;
		}
		for (j = 0; j < height; ++j) {
			p_s = cv_mat.data + (j * width) * 3;
			for (i = 0; i < width; ++i) {
				h_i = i * 3;
				p_d_r[0] = p_s[h_i + 2];
				p_d_g[0] = p_s[h_i + 1];
				p_d_b[0] = p_s[h_i];
				p_d_r++;
				p_d_g++;
				p_d_b++;
			}
			p_d_r += pitch - width;
			p_d_g += pitch - width;
			p_d_b += pitch - width;
		}
	} while (0);

	return rval;
}

int save_nv12_to_file(uint8_t *y, uint8_t *uv,
	uint32_t h, uint32_t w, uint32_t p, std::string fn)
{
	uint8_t *nv12 = NULL;
	int rval = 0;
	do {
		nv12 = (uint8_t*)malloc(h * w * sizeof(uint8_t) * 3 >> 1);
		if (nv12 == NULL) {
			printf("allocate nv12 buffer error !\n");
			rval = -1;
			break;
		}
		memset(nv12, 0, h * w * sizeof(uint8_t) * 3 >> 1);
		cv::Mat img_full = convert_yuv_to_bgr24(nv12, y, uv, w,	h, p);
		cv::imwrite(fn, img_full);
		free(nv12);
	} while (0);

	return rval;
}

void save_rgb_to_file(int h, int w, int t, uint8_t *addr,
	int is_bgr, int is_padding, std::string fn, int idx)
{
	cv::Mat img(h, w, t);
	convert_rgb_to_mat(img, addr, is_bgr, is_padding);
	std::stringstream filename_resize;
	filename_resize << fn << idx << ".jpg";
	cv::imwrite(filename_resize.str(), img);

	return;
}

void save_mat_to_file(cv::Mat img, std::string fn, int idx)
{
	std::stringstream filename;
	filename << fn << idx << ".jpg";
	cv::imwrite(filename.str(), img);

	return;
}

unsigned long gettimeus(void)
{
	static struct timeval tv;

	gettimeofday(&tv, NULL);

	return (unsigned long) 1000000 * (unsigned long) (tv.tv_sec) + (unsigned long) (tv.tv_usec);
}

