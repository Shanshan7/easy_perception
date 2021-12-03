/*******************************************************************************
 * kalman_filter.h
 *
 * History:
 *  2020/12/22 - [Du You] create file
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

#ifndef KALMAN_FILTER_H_
#define KALMAN_FILTER_H_

#include <cstddef>
#include <vector>
#include <Eigen/Core>

#include <mot.h>	// MOT_REID_DIM is defined here

#define DET_N_DIM						(4)
#define REID_FEATURE_DIM				(MOT_REID_DIM)

typedef Eigen::Matrix<float, 1,             DET_N_DIM * 2, Eigen::RowMajor>
kf_mean;
typedef Eigen::Matrix<float, DET_N_DIM * 2, DET_N_DIM * 2, Eigen::RowMajor>
kf_covariance;
typedef Eigen::Matrix<float, 1,             DET_N_DIM,     Eigen::RowMajor>
kf_mean_pos;
typedef Eigen::Matrix<float, DET_N_DIM,     DET_N_DIM,     Eigen::RowMajor>
kf_covariance_pos;
using kf_data = std::pair<kf_mean, kf_covariance>;
using kf_data_pos = std::pair<kf_mean_pos, kf_covariance_pos>;

typedef Eigen::Matrix<float, 1,                   DET_N_DIM, Eigen::RowMajor>
detection_box;
typedef Eigen::Matrix<float, Eigen::Dynamic,      DET_N_DIM, Eigen::RowMajor>
detection_boxes;
typedef Eigen::Matrix<float, 1,                   REID_FEATURE_DIM,
Eigen::RowMajor> reid_feature;
typedef Eigen::Matrix<float, Eigen::Dynamic, REID_FEATURE_DIM, Eigen::RowMajor>
reid_features;
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
dynamic_matrix;

class kalman_filter
{
public:
	static const float chi2inv95[10];

	kalman_filter(float dt = 1.0);
	~kalman_filter() {};

	kf_data initiate(const detection_box &measurement);
	void predict(kf_mean &mean, kf_covariance &covariance);
	kf_data_pos
	project(const kf_mean &mean, const kf_covariance &covariance);
	kf_data update(const kf_mean &mean,
		const kf_covariance &covariance,
		const detection_box &measurement);
	void set_dt(float dt);

private:
	Eigen::Matrix<float, DET_N_DIM * 2, DET_N_DIM * 2, Eigen::RowMajor>
		m_motion_mat;
	Eigen::Matrix<float, DET_N_DIM,     DET_N_DIM * 2, Eigen::RowMajor>
		m_update_mat;
	float m_std_weight_position;
	float m_std_weight_velocity;
};

#endif // KALMAN_FILTER_H_
