/*******************************************************************************
 * utils.hpp
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
#ifndef __SSD_LPR_UTILS_H__
#define __SSD_LPR_UTILS_H__

// Depends on <opencv2/opencv.hpp>

void convert_rgb_to_mat(cv::Mat &img, unsigned char *bin, int is_bgr,
	int padding);
cv::Mat convert_yuv_to_bgr24(uint8_t *nv12_buffer, uint8_t *y_addr,
	uint8_t *uv_addr, int width, int height, int pitch);
int convert_cvMat_to_rgb(cv::Mat cv_mat, uint8_t *rgb, int is_bgr,
	int padding);
int save_nv12_to_file(uint8_t *y, uint8_t *uv, uint32_t h, uint32_t w,
	uint32_t p, std::string fn);
void save_rgb_to_file(int h, int w, int t, uint8_t *addr, int is_bgr,
	int is_padding, std::string fn, int idx);
void save_mat_to_file(cv::Mat img, std::string fn, int idx);
unsigned long gettimeus(void);
#endif

