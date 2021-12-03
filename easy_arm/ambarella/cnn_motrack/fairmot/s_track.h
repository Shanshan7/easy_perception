/*******************************************************************************
 * s_track.h
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

#ifndef S_TRACK_H_
#define S_TRACK_H_

#include <vector>
#include <deque>
#include <memory>

#include "kalman_filter.h"

#define S_TRACK_USE_DET_RESULT_FOR_TLWH	0

enum s_track_state { init, tracked, lost, removed };

class s_track
{
public:
	static int next_track_id() { return track_id_count++; };
	static int next_tentative_id() { return tentative_id_count++; };

private:
	static int track_id_count;
	static int tentative_id_count;

public:
	s_track(detection_box &tlwh, float score, reid_feature &feature,
		int image_width, int image_height);
	~s_track() {};

	void predict();
	void activate(std::shared_ptr<kalman_filter> &kf, int frame_id,
		bool use_smooth_feature = false,
		int feature_buffer_size = 5,
		float smooth_alpha = 0.9f,
		int tentative_thresh = 3);
	void reactivate(const std::shared_ptr<s_track> &detection, int frame_id,
		bool use_new_id = false);
	void update(const std::shared_ptr<s_track> &detection, int frame_id,
		bool update_feature = true);
	Eigen::Matrix<float, 1, -1> gating_distance(
		const std::vector<std::shared_ptr<s_track>> &detections) const;
	Eigen::Matrix<float, 1, -1> embedding_distance(
		const std::vector<std::shared_ptr<s_track>> &detections) const;
	Eigen::Matrix<float, 1, -1> iou_distance(
		const std::vector<std::shared_ptr<s_track>> &detections) const;
	detection_box tlwh() const { return m_tlwh; }
	detection_box to_normalized_tlwh() const;
	detection_box to_tlbr() const;
	detection_box to_xyah() const;
	detection_box det_tlwh() const { return m_det_tlwh; }
	detection_box to_normalized_det_tlwh() const;
	detection_box to_det_tlbr() const;
	detection_box to_det_xyah() const;
	int track_id() const { return m_track_id; }
	int tentative_id() const { return m_tentative_id; }
	int frame_id() const { return m_frame_id; }
	int start_frame_id() const { return m_start_frame_id; }
	const reid_feature &smooth_feature() const { return m_smooth_feature; }
	const reid_feature &cur_feature() const { return m_cur_feature; }
	s_track_state state() const { return m_state; }
	void set_state(s_track_state state);

private:
	void unitize_feature(reid_feature &feature);
	void update_features(reid_feature &feature);
	void update_tlwh(const std::shared_ptr<s_track> &detection);

private:
	std::shared_ptr<kalman_filter> m_kf;
	detection_box m_tlwh;
	detection_box m_det_tlwh;
	float m_score;
	int m_image_width;
	int m_image_height;
	kf_mean m_mean;
	kf_covariance m_covariance;
	reid_feature m_cur_feature;
	reid_feature m_smooth_feature;
	bool m_feature_updated;
	int m_frame_id;
	int m_start_frame_id;
	std::deque<reid_feature> m_features;
	bool m_use_smooth_feature;
	float m_smooth_alpha;
	int m_feature_buffer_size;
	int m_tentative_thresh;

	int m_track_id;
	int m_tentative_id;
	int m_tentative_count;
	s_track_state m_state;
};

typedef std::shared_ptr<s_track> s_track_ptr;

#endif // S_TRACK_H_
