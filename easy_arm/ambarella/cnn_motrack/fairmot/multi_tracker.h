/*******************************************************************************
 * tracker.h
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

#ifndef MULTI_TRACKER_H_
#define MULTI_TRACKER_H_

#include <vector>

#include "kalman_filter.h"
#include "s_track.h"

#define INFINITY_COST					(1e5f)
#define MAX_REMOVED_TRACK_QUEUE_SIZE	(100)

class multi_tracker
{
public:
	multi_tracker(
		int image_width,
		int image_height,
		bool embedding_reliable_use_iou,
		float embedding_reliable_distance,
		float embedding_cosine_distance,
		bool overlap_reliable_use_iou,
		float overlap_reliable_distance,
		float overlap_iou_distance,
		bool confirming_use_iou,
		float confirming_distance,
		float duplicate_iou_distance,
		int max_lost_age = 15,
		float fuse_lambda = 0.99f,
		bool use_match_priority = true,
		bool merge_match_priority = false,
		int match_priority_thresh = 3,
		float dt = 1.0f,
		bool use_smooth_feature = false,
		int feature_buffer_size = 5,
		float smooth_alpha = 0.9f,
		int tentative_thresh = 3,
		bool verbose = false);
	~multi_tracker() {};

	void update(std::vector<s_track_ptr> &new_tracks);

	int frame_id() const { return m_frame_id; }
	const std::vector<s_track_ptr> &available_tracks() const
	{ return m_available; }

private:
	dynamic_matrix embedding_distance(const std::vector<s_track_ptr> &tracks,
		const std::vector<s_track_ptr> &detections);
	dynamic_matrix iou_distance(const std::vector<s_track_ptr> &tracks,
		const std::vector<s_track_ptr> &detections);
	void fuse_motion(dynamic_matrix &distances,
		const std::vector<s_track_ptr> &tracks,
		const std::vector<s_track_ptr> &detections,
		float fuse_lambda);
	void linear_assignment(const dynamic_matrix &distances,
		float thresh,
		std::vector<std::pair<int, int>> &matched_pairs,
		std::vector<int> &unmatched_track_indexes,
		std::vector<int> &unmatched_detection_indexes);
	enum distance_type {embedding, iou};
	void match_with_min_cost(distance_type type, float thresh,
		const std::vector<s_track_ptr> &tracks,
		const std::vector<s_track_ptr> &detections,
		std::vector<std::pair<int, int>> &matched_pairs,
		std::vector<int> &unmatched_track_indexes,
		std::vector<int> &unmatched_detection_indexes);
	void match_with_priority(distance_type type, float thresh,
		const std::vector<s_track_ptr> &tracks,
		const std::vector<s_track_ptr> &detections,
		std::vector<std::pair<int, int>> &matched_pairs,
		std::vector<int> &unmatched_track_indexes,
		std::vector<int> &unmatched_detection_indexes);
	void remove_unreliable_detections(distance_type type, float thresh,
		const std::vector<s_track_ptr> &tracks,
		const std::vector<s_track_ptr> &detections,
		std::vector<s_track_ptr> &reliable,
		std::vector<s_track_ptr> &unreliable);
	std::vector<s_track_ptr> joint_tracks(
		const std::vector<s_track_ptr> &a,
		const std::vector<s_track_ptr> &b);
	std::vector<s_track_ptr> choose_tracks(
		const std::vector<s_track_ptr> &tracks,
		bool (*cond)(const s_track_ptr &));
	std::vector<s_track_ptr> sub_tracks(
		const std::vector<s_track_ptr> &a,
		const std::vector<s_track_ptr> &b);
	std::vector<s_track_ptr> remove_duplicate_tracks(
		std::vector<s_track_ptr> &a,
		std::vector<s_track_ptr> &b);

private:
	std::shared_ptr<kalman_filter> m_kf;
	std::vector<s_track_ptr> m_tracked;
	std::vector<s_track_ptr> m_lost;
	std::vector<s_track_ptr> m_removed;
	std::vector<s_track_ptr> m_available;
	int m_frame_id;

	int m_image_width;
	int m_image_height;
	bool m_embedding_reliable_use_iou;
	float m_embedding_reliable_distance;
	float m_embedding_cosine_distance;
	bool m_overlap_reliable_use_iou;
	float m_overlap_reliable_distance;
	float m_overlap_iou_distance;
	bool m_confirming_use_iou;
	float m_confirming_distance;
	float m_duplicate_iou_distance;
	int m_max_lost_age;
	float m_fuse_lambda;
	bool m_use_match_priority;
	bool m_merge_match_priority;
	int m_match_priority_thresh;
	float m_dt;
	int m_feature_buffer_size;
	bool m_use_smooth_feature;
	float m_smooth_alpha;
	int m_tentative_thresh;
	bool m_verbose;
};

#endif // MULTI_TRACKER_H_
