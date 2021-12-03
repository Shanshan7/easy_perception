/*******************************************************************************
 * kalman_filter.cpp
 *
 * History:
 *  2020/12/22  - [Du You] created
 *
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

#include <Eigen/Cholesky>

#include "kalman_filter.h"

const float kalman_filter::chi2inv95[10] = {
	3.8415,
	5.9915,
	7.8147,
	9.4877,
	11.070,
	12.592,
	14.067,
	15.507,
	16.919
};

kalman_filter::kalman_filter(float dt)
{
	m_motion_mat = Eigen::MatrixXf::Identity(DET_N_DIM * 2, DET_N_DIM * 2);
	for (int i = 0; i < DET_N_DIM; i++) {
		m_motion_mat(i, DET_N_DIM + i) = dt;
	}

	m_update_mat = Eigen::MatrixXf::Identity(DET_N_DIM, DET_N_DIM * 2);
	m_std_weight_position = 1.0 / 20;
	m_std_weight_velocity = 1.0 / 160;
}

kf_data kalman_filter::initiate(const detection_box &measurement)
{
	kf_mean mean;
	kf_mean std_mean;
	kf_covariance covariance;

	for (int i = 0; i < DET_N_DIM * 2; i++) {
		if(i < 4) {
			mean(i) = measurement(i);
		} else {
			mean(i) = 0.0;
		}
	}

	std_mean(0) = 2 * m_std_weight_position * measurement(3);
	std_mean(1) = 2 * m_std_weight_position * measurement(3);
	std_mean(2) = 1e-2;
	std_mean(3) = 2 * m_std_weight_position * measurement(3);
	std_mean(4) = 10 * m_std_weight_velocity * measurement(3);
	std_mean(5) = 10 * m_std_weight_velocity * measurement(3);
	std_mean(6) = 1e-5;
	std_mean(7) = 10 * m_std_weight_velocity * measurement(3);

	covariance = std_mean.array().square().matrix().asDiagonal();

	return std::make_pair(mean, covariance);
}

void kalman_filter::predict(kf_mean &mean, kf_covariance &covariance)
{
	detection_box std_pos;
	detection_box std_vel;
	kf_mean std_mean;
	kf_covariance motion_cov;

	std_mean(0) = m_std_weight_position * mean(3);
	std_mean(1) = m_std_weight_position * mean(3);
	std_mean(2) = 1e-2;
	std_mean(3) = m_std_weight_position * mean(3);
	std_mean(4) = m_std_weight_velocity * mean(3);
	std_mean(5) = m_std_weight_velocity * mean(3);
	std_mean(6) = 1e-5;
	std_mean(7) = m_std_weight_velocity * mean(3);

	mean = m_motion_mat * mean.transpose();
	motion_cov = std_mean.array().square().matrix().asDiagonal();
	covariance =
		m_motion_mat * covariance * (m_motion_mat.transpose()) + motion_cov;
}

kf_data_pos
kalman_filter::project(const kf_mean &mean, const kf_covariance &covariance)
{
	detection_box std_pos;

	std_pos(0) = m_std_weight_position * mean(3);
	std_pos(1) = m_std_weight_position * mean(3);
	std_pos(2) = 1e-1;
	std_pos(3) = m_std_weight_position * mean(3);

	kf_mean_pos mean_pos = m_update_mat * mean.transpose();
	kf_covariance_pos innovation_cov =
		std_pos.array().square().matrix().asDiagonal();
	kf_covariance_pos covariance_pos =
		m_update_mat * covariance * (m_update_mat.transpose()) + innovation_cov;

	return std::make_pair(mean_pos, covariance_pos);
}

kf_data kalman_filter::update(const kf_mean &mean,
	const kf_covariance &covariance,
	const detection_box &measurement)
{
	kf_data_pos projected_data = project(mean, covariance);
	kf_mean_pos projected_mean = projected_data.first;
	kf_covariance_pos projected_cov = projected_data.second;

	Eigen::Matrix<float, DET_N_DIM,	 DET_N_DIM * 2> factor =
		(covariance * (m_update_mat.transpose())).transpose();
	Eigen::Matrix<float, DET_N_DIM * 2, DET_N_DIM> kalman_gain =
		(projected_cov.llt().solve(factor)).transpose();
	kf_mean_pos innovation = measurement - projected_mean;

	kf_mean new_mean = (mean.array() +
		(innovation * kalman_gain.transpose()).array()).matrix();
	kf_covariance new_covariance = covariance -
		kalman_gain * projected_cov * (kalman_gain.transpose());
	return std::make_pair(new_mean, new_covariance);
}

void kalman_filter::set_dt(float dt)
{
	for (int i = 0; i < DET_N_DIM; i++) {
		m_motion_mat(i, DET_N_DIM + i) = dt;
	}
}
