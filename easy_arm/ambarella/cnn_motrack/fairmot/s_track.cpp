/*******************************************************************************
 * s_track.cpp
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
#include "s_track.h"

int s_track::track_id_count = 0;
int s_track::tentative_id_count = 0;

s_track::s_track(detection_box &tlwh, float score, reid_feature &feature,
	int image_width, int image_height)
{
	m_kf = NULL;
	m_tlwh = tlwh;
	m_det_tlwh = tlwh;
	m_score = score;
	m_image_width = image_width;
	m_image_height = image_height;
	m_tentative_count = 0;
	m_tentative_id = -1;
	m_feature_updated = false;
	m_frame_id = -1;
	m_start_frame_id = -1;
	m_use_smooth_feature = false;
	m_smooth_alpha = 0.9f;
	m_feature_buffer_size = 1;
	m_tentative_thresh = 3;
	m_track_id = -1;
	m_tentative_count = 0;
	update_features(feature);
	m_state = s_track_state::init;
}

void s_track::predict()
{
	assert(m_kf != NULL);

	kf_mean mean_state = m_mean;
	if (m_state != s_track_state::tracked) {
		mean_state(7) = 0.0f;
	}

	m_kf->predict(mean_state, m_covariance);
	m_mean = mean_state;
}

void s_track::activate(std::shared_ptr<kalman_filter> &kf, int frame_id,
	bool use_smooth_feature,
	int feature_buffer_size,
	float smooth_alpha,
	int tentative_thresh)
{
	m_kf = kf;
	m_track_id = -1;
	kf_data data = m_kf->initiate(to_xyah());
	m_mean = data.first;
	m_covariance = data.second;
	m_frame_id = frame_id;
	m_start_frame_id = frame_id;
	m_use_smooth_feature = use_smooth_feature;
	m_feature_buffer_size = feature_buffer_size;
	m_smooth_alpha = smooth_alpha;
	m_tentative_thresh = tentative_thresh;
	assert(m_feature_buffer_size > 0);
	m_tentative_count++;
	m_tentative_id = next_tentative_id();
	if (frame_id == 1) {
		m_state = s_track_state::tracked;
		m_track_id = next_track_id();
	}
}

void s_track::update_tlwh(const std::shared_ptr<s_track> &detection)
{
#if S_TRACK_USE_DET_RESULT_FOR_TLWH
	m_tlwh = detection->tlwh();
	m_det_tlwh = detection->tlwh();
#else
	m_tlwh(2) = m_mean(2) * m_mean(3);
	m_tlwh(3) = m_mean(3);
	m_tlwh(0) = m_mean(0) - m_tlwh(2) / 2;
	m_tlwh(1) = m_mean(1) - m_tlwh(3) / 2;
	m_det_tlwh = detection->tlwh();
#endif
}

void s_track::reactivate(const std::shared_ptr<s_track> &detection,
	int frame_id, bool use_new_id)
{
	assert(m_kf != NULL);

	kf_data data = m_kf->update(m_mean, m_covariance, detection->to_xyah());
	m_mean = data.first;
	m_covariance = data.second;
	update_tlwh(detection);
	update_features(detection->m_cur_feature);
	m_frame_id = frame_id;
	if (use_new_id) {
		m_track_id = next_track_id();
	}

	m_state = s_track_state::tracked;
}

void s_track::set_state(s_track_state state)
{
	switch (state) {
	case s_track_state::init:
		break;
	case s_track_state::tracked:
		break;
	case s_track_state::lost:
		break;
	case s_track_state::removed:
		break;
	default :
		assert(0);
		break;
	}

	m_state = state;
}

void s_track::update(const std::shared_ptr<s_track> &detection,
	int frame_id, bool update_feature)
{
	assert(m_kf != NULL);

	m_frame_id = frame_id;
	kf_data data = m_kf->update(m_mean, m_covariance, detection->to_xyah());
	m_mean = data.first;
	m_covariance = data.second;
	update_tlwh(detection);
	m_score = detection->m_score;
	if (update_feature) {
		update_features(detection->m_cur_feature);
	}

	if (m_tentative_count < m_tentative_thresh) {
		m_tentative_count++;
	}
	else {
		m_state = s_track_state::tracked;
		if (m_track_id == -1) {
			m_track_id = next_track_id();
		}
	}
}

Eigen::Matrix<float, 1, -1> s_track::gating_distance(
	const std::vector<s_track_ptr> &detections) const
{
	kf_data_pos projected_data = m_kf->project(m_mean, m_covariance);
	kf_mean_pos projected_mean = projected_data.first;
	kf_covariance_pos projected_cov = projected_data.second;
	detection_boxes d(detections.size(), DET_N_DIM);
	int i = 0;

	for(const s_track_ptr &track : detections) {
		d.row(i++) = track->to_xyah() - projected_mean;
	}

	Eigen::Matrix<float, -1, -1, Eigen::RowMajor> factor =
		projected_cov.llt().matrixL();
	Eigen::Matrix<float, -1, -1> z = factor.triangularView<Eigen::Lower>().
		solve<Eigen::OnTheRight>(d).transpose();
	auto squared_maha = ((z.array())*(z.array())).matrix().colwise().sum();
	return squared_maha;
}

Eigen::Matrix<float, 1, -1> s_track::embedding_distance(
	const std::vector<std::shared_ptr<s_track>> &detections) const
{
	if (m_use_smooth_feature) {
		reid_features track_features(1, REID_FEATURE_DIM);
		reid_features detection_features(detections.size(), REID_FEATURE_DIM);
		dynamic_matrix ret;
		int row;

		track_features.row(0) = m_smooth_feature;

		row = 0;
		for (const s_track_ptr &detection : detections) {
			detection_features.row(row++) = detection->cur_feature();
		}

		ret = 1.0f - (track_features * detection_features.transpose()).array();
		ret = ret.array().max(
			dynamic_matrix::Zero(ret.rows(), ret.cols()).array());
		return ret;
	} else {
		reid_features track_features(m_features.size(), REID_FEATURE_DIM);
		reid_features detection_features(detections.size(), REID_FEATURE_DIM);
		dynamic_matrix ret;
		int row;

		row = 0;
		for (const reid_feature &feature : m_features) {
			track_features.row(row++) = feature;
		}

		row = 0;
		for (const s_track_ptr &detection : detections) {
			detection_features.row(row++) = detection->cur_feature();
		}

		ret = 1.0f - (track_features * detection_features.transpose()).array();
		ret = ret.array().max(dynamic_matrix::Zero(ret.rows(),
			ret.cols()).array());

		return ret.colwise().minCoeff().transpose();
	}
}

Eigen::Matrix<float, 1, -1> s_track::iou_distance(
	const std::vector<std::shared_ptr<s_track>> &detections) const
{
	detection_box box = to_tlbr();
	float area = m_tlwh(2) * m_tlwh(3);

	Eigen::Matrix<float, 1, -1> ret = Eigen::MatrixXf::Zero(1,
		detections.size());

	for (size_t i = 0; i < detections.size(); i++) {
		detection_box det_box = detections[i]->to_tlbr();
		float det_area = detections[i]->tlwh()(2) * detections[i]->tlwh()(3);
		float inter_x_start = std::max(det_box(0), box(0));
		float inter_y_start = std::max(det_box(1), box(1));
		float inter_x_end = std::min(det_box(2), box(2));
		float inter_y_end = std::min(det_box(3), box(3));

		float inter_w = std::max(0.0f, inter_x_end - inter_x_start);
		float inter_h = std::max(0.0f, inter_y_end - inter_y_start);
		float inter_area = inter_w * inter_h;
		ret(0, i) = inter_area / (area + det_area - inter_area
			+ std::numeric_limits<float>::min());
	}

	return 1.0f - ret.array();
}

detection_box s_track::to_normalized_tlwh() const
{
	detection_box ret = m_tlwh;

	ret(0, 0) = ret(0, 0) / m_image_width;
	ret(0, 1) = ret(0, 1) / m_image_height;
	ret(0, 2) = ret(0, 2) / m_image_width;
	ret(0, 3) = ret(0, 3) / m_image_height;

	return ret;
}

detection_box s_track::to_tlbr() const
{
	detection_box ret = m_tlwh;

	ret(0, 2) = ret(0, 0) + ret(0, 2);
	ret(0, 3) = ret(0, 1) + ret(0, 3);

	return ret;
}

detection_box s_track::to_xyah() const
{
	detection_box ret = m_tlwh;

	ret(0, 0) += (ret(0, 2)*0.5);
	ret(0, 1) += (ret(0, 3)*0.5);
	ret(0, 2) /= ret(0, 3);

	return ret;
}

detection_box s_track::to_normalized_det_tlwh() const
{
	detection_box ret = m_det_tlwh;

	ret(0, 0) = ret(0, 0) / m_image_width;
	ret(0, 1) = ret(0, 1) / m_image_height;
	ret(0, 2) = ret(0, 2) / m_image_width;
	ret(0, 3) = ret(0, 3) / m_image_height;

	return ret;
}

detection_box s_track::to_det_tlbr() const
{
	detection_box ret = m_det_tlwh;

	ret(0, 2) = ret(0, 0) + ret(0, 2);
	ret(0, 3) = ret(0, 1) + ret(0, 3);

	return ret;
}

detection_box s_track::to_det_xyah() const
{
	detection_box ret = m_det_tlwh;

	ret(0, 0) += (ret(0, 2)*0.5);
	ret(0, 1) += (ret(0, 3)*0.5);
	ret(0, 2) /= ret(0, 3);

	return ret;
}

void s_track::unitize_feature(reid_feature &feature)
{
	float norm = 0.0f;

	for (int i = 0; i < feature.cols(); i++) {
		norm += pow(feature(i), 2);
	}

	norm = sqrt(norm);

	for (int i = 0; i < feature.cols(); i++) {
		feature(i) = feature(i) / norm;
	}
}

void s_track::update_features(reid_feature &feature)
{
	reid_feature feat = feature;
	unitize_feature(feat);
	m_cur_feature = feat;
	if (!m_feature_updated) {
		m_smooth_feature = feat;
		m_feature_updated = true;
	} else {
		m_smooth_feature = m_smooth_feature.array() * m_smooth_alpha +
			feat.array() * (1.0f - m_smooth_alpha);
	}

	m_features.push_back(feat);
	if (m_features.size() > (size_t)m_feature_buffer_size) {
		m_features.pop_front();
	}

	unitize_feature(m_smooth_feature);
}
