/*******************************************************************************
 * tracker.cpp
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

#include <limits>
#include <cmath>

#include "tracking_log.h"
#include "hungarian_wrap.h"
#include "multi_tracker.h"

using namespace std;

multi_tracker::multi_tracker(
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
	int max_lost_age,
	float fuse_lambda,
	bool use_match_priority,
	bool merge_match_priority,
	int match_priority_thresh,
	float dt,
	bool use_smooth_feature,
	int feature_buffer_size,
	float smooth_alpha,
	int tentative_thresh,
	bool verbose)
{
	m_kf = std::shared_ptr<kalman_filter>(new kalman_filter(dt));
	m_frame_id = 0;
	m_image_width = image_width;
	m_image_height = image_height;
	m_embedding_reliable_use_iou = embedding_reliable_use_iou;
	m_embedding_reliable_distance = embedding_reliable_distance;
	m_embedding_cosine_distance = embedding_cosine_distance;
	m_overlap_reliable_use_iou = overlap_reliable_use_iou;
	m_overlap_reliable_distance = overlap_reliable_distance;
	m_overlap_iou_distance = overlap_iou_distance;
	m_confirming_use_iou = confirming_use_iou;
	m_confirming_distance = confirming_distance;
	m_duplicate_iou_distance = duplicate_iou_distance;
	m_max_lost_age = max_lost_age;
	m_fuse_lambda = fuse_lambda;
	m_use_match_priority = use_match_priority;
	m_merge_match_priority = merge_match_priority;
	m_match_priority_thresh = match_priority_thresh;
	m_dt = dt;
	m_use_smooth_feature = use_smooth_feature;
	m_feature_buffer_size = feature_buffer_size;
	m_smooth_alpha = smooth_alpha;
	m_tentative_thresh = tentative_thresh;
	m_verbose = verbose;

	tracking_log::enable(verbose);

	tracking_log::out() << "multi tracker:" << std::endl;
	tracking_log::out() << "m_image_width " << m_image_width << std::endl;
	tracking_log::out() << "m_image_height " << m_image_height << std::endl;
	tracking_log::out() << "m_embedding_reliable_use_iou " << m_embedding_reliable_use_iou << std::endl;
	tracking_log::out() << "m_embedding_reliable_distance " << m_embedding_reliable_distance << std::endl;
	tracking_log::out() << "m_embedding_cosine_distance " << m_embedding_cosine_distance << std::endl;
	tracking_log::out() << "m_overlap_reliable_use_iou " << m_overlap_reliable_use_iou << std::endl;
	tracking_log::out() << "m_overlap_reliable_distance " << m_overlap_reliable_distance << std::endl;
	tracking_log::out() << "m_overlap_iou_distance " << m_overlap_iou_distance << std::endl;
	tracking_log::out() << "m_confirming_use_iou " << m_confirming_use_iou << std::endl;
	tracking_log::out() << "m_confirming_distance " << m_confirming_distance << std::endl;
	tracking_log::out() << "m_duplicate_iou_distance " << m_duplicate_iou_distance << std::endl;
	tracking_log::out() << "m_max_lost_age " << m_max_lost_age << std::endl;
	tracking_log::out() << "m_fuse_lambda " << m_fuse_lambda << std::endl;
	tracking_log::out() << "m_use_match_priority " << m_use_match_priority << std::endl;
	tracking_log::out() << "m_merge_match_priority " << m_merge_match_priority << std::endl;
	tracking_log::out() << "m_match_priority_thresh " << m_match_priority_thresh << std::endl;
	tracking_log::out() << "m_dt " << m_dt << std::endl;
	tracking_log::out() << "m_use_smooth_feature " << m_use_smooth_feature << std::endl;
	tracking_log::out() << "m_feature_buffer_size " << m_feature_buffer_size << std::endl;
	tracking_log::out() << "m_smooth_alpha " << m_smooth_alpha << std::endl;
	tracking_log::out() << "m_tentative_thresh " << m_tentative_thresh << std::endl;
	tracking_log::out() << "m_verbose " << m_verbose << std::endl;
}

void multi_tracker::update(std::vector<s_track_ptr> &detections)
{
	std::vector<s_track_ptr> embedding_reliable_detections;
	std::vector<s_track_ptr> embedding_unreliable_detections;
	std::vector<s_track_ptr> overlap_reliable_detections;
	std::vector<s_track_ptr> overlap_unreliable_detections;
	std::vector<s_track_ptr> tracked_tracks;
	std::vector<s_track_ptr> unconfirmed_tracks;
	std::vector<s_track_ptr> embedding_tracked_tracks;
	std::vector<s_track_ptr> overlap_tracked_tracks;
	std::vector<s_track_ptr> activated_tracks;
	std::vector<s_track_ptr> refined_tracks;
	std::vector<s_track_ptr> tentative_tracks;
	std::vector<s_track_ptr> new_tracks;
	std::vector<s_track_ptr> lost_tracks;
	std::vector<s_track_ptr> removed_tracks;
	std::vector<s_track_ptr> removed_duplicate;
	std::vector<s_track_ptr> unmatched_detections;
	dynamic_matrix distance_matrix;
	std::vector<std::pair<int, int>> matched_pairs;
	std::vector<int> unmatched_track_indexes;
	std::vector<int> unmatched_detection_indexes;
	distance_type dist_type = distance_type::embedding;

	tracking_log::out() << "Update begin ++++++++++" << std::endl;

	m_frame_id++;
	tracking_log::out() << "FRAME " << m_frame_id << std::endl;

	for (s_track_ptr &track : m_tracked) {
		track->predict();
	}

	for (s_track_ptr &track : m_lost) {
		track->predict();
	}

	for (s_track_ptr &track : m_tracked) {
		if (track->state() == s_track_state::tracked
			|| track->state() == s_track_state::lost) {
			tracked_tracks.push_back(track);
		} else {
			unconfirmed_tracks.push_back(track);
		}
	}

	embedding_tracked_tracks = joint_tracks(tracked_tracks, m_lost);

	// Step 2: First association, with embedding
	tracking_log::out() << std::endl << "--EMBEDDING--" << std::endl;
	tracking_log::print_tracks(embedding_tracked_tracks, "Tracks:");
	tracking_log::print_tracks(detections, "Detections:");

	if (m_embedding_reliable_distance > 0.0f) {
		if (m_embedding_reliable_use_iou) {
			tracking_log::out() << "Reliable use IOU" << std::endl;
			dist_type = distance_type::iou;
		} else {
			tracking_log::out() << "Reliable use cosine" << std::endl;
			dist_type = distance_type::embedding;
		}

		remove_unreliable_detections(dist_type, m_embedding_reliable_distance,
				embedding_tracked_tracks, detections,
				embedding_reliable_detections, embedding_unreliable_detections);
		tracking_log::print_tracks(embedding_reliable_detections, "Reliable:");
		tracking_log::print_tracks(embedding_unreliable_detections, "Unreliable:");
	}
	else {
		embedding_reliable_detections = detections;
	}

	if (m_use_match_priority) {
		match_with_priority(distance_type::embedding,
			m_embedding_cosine_distance,
			embedding_tracked_tracks, embedding_reliable_detections,
			matched_pairs,
			unmatched_track_indexes, unmatched_detection_indexes);
	} else {
		match_with_min_cost(distance_type::embedding,
			m_embedding_cosine_distance,
			embedding_tracked_tracks, embedding_reliable_detections,
			matched_pairs,
			unmatched_track_indexes, unmatched_detection_indexes);
	}

	tracking_log::print_matched(embedding_tracked_tracks,
		embedding_reliable_detections, matched_pairs, "MATCHED:");
	for (const std::pair<int, int> &p : matched_pairs) {
		s_track_ptr &track = embedding_tracked_tracks[p.first];
		s_track_ptr &detection = embedding_reliable_detections[p.second];
		if (track->state() == s_track_state::tracked) {
			track->update(detection, m_frame_id);
			activated_tracks.push_back(track);
		} else {
			track->reactivate(detection, m_frame_id);
			refined_tracks.push_back(track);
		}
	}

	// Step 3: Second association, with IOU
	tracking_log::out() << std::endl << "--IOU--" << std::endl;
	for (int i : unmatched_track_indexes) {
		if (embedding_tracked_tracks[i]->state() == s_track_state::tracked) {
			overlap_tracked_tracks.push_back(embedding_tracked_tracks[i]);
		}
	}

	unmatched_detections.clear();
	for (int i : unmatched_detection_indexes) {
		unmatched_detections.push_back(embedding_reliable_detections[i]);
	}

	tracking_log::print_tracks(overlap_tracked_tracks, "Tracks:");
	tracking_log::print_tracks(unmatched_detections, "Detections:");
	if (m_overlap_reliable_distance > 0.0f) {
		if (m_overlap_reliable_use_iou) {
			tracking_log::out() << "Reliable use IOU" << std::endl;
			dist_type = distance_type::iou;
		} else {
			tracking_log::out() << "Reliable use cosine" << std::endl;
			dist_type = distance_type::embedding;
		}

		remove_unreliable_detections(dist_type, m_overlap_reliable_distance,
				overlap_tracked_tracks, unmatched_detections,
				overlap_reliable_detections, overlap_unreliable_detections);
		tracking_log::print_tracks(overlap_reliable_detections, "Reliable:");
		tracking_log::print_tracks(overlap_unreliable_detections, "Unreliable:");
	} else {
		overlap_reliable_detections = unmatched_detections;
	}

	match_with_min_cost(distance_type::iou, m_overlap_iou_distance,
		overlap_tracked_tracks, overlap_reliable_detections,
		matched_pairs, unmatched_track_indexes, unmatched_detection_indexes);
	tracking_log::print_matched(overlap_tracked_tracks,
		overlap_reliable_detections, matched_pairs, "MATCHED:");
	for (const std::pair<int, int> &p : matched_pairs) {
		s_track_ptr &track = overlap_tracked_tracks[p.first];
		s_track_ptr &detection = overlap_reliable_detections[p.second];
		if (track->state() == s_track_state::tracked) {
			track->update(detection, m_frame_id);
			activated_tracks.push_back(track);
		} else {
			track->reactivate(detection, m_frame_id);
			refined_tracks.push_back(track);
		}
	}

	for (int i : unmatched_track_indexes) {
		if (overlap_tracked_tracks[i]->state() != s_track_state::lost) {
			overlap_tracked_tracks[i]->set_state(s_track_state::lost);
			lost_tracks.push_back(overlap_tracked_tracks[i]);
		}
	}

	// Deal with unconfirmed tracks
	tracking_log::out() << std::endl << "--CONFIRMING--" << std::endl;
	unmatched_detections.clear();
	for (int i : unmatched_detection_indexes) {
		unmatched_detections.push_back(overlap_reliable_detections[i]);
	}

	tracking_log::print_tracks(unconfirmed_tracks, "Tracks:");
	tracking_log::print_tracks(unmatched_detections, "Detections:");
	if (m_confirming_use_iou) {
		tracking_log::out() << "Confirming use IOU" << std::endl;
		dist_type = distance_type::iou;
	} else {
		tracking_log::out() << "Confirming use cosine" << std::endl;
		dist_type = distance_type::embedding;
	}

	match_with_min_cost(dist_type, m_confirming_distance,
		unconfirmed_tracks, unmatched_detections,
		matched_pairs, unmatched_track_indexes, unmatched_detection_indexes);
	for (const std::pair<int, int> &p : matched_pairs) {
		s_track_ptr &track = unconfirmed_tracks[p.first];
		s_track_ptr &detection = unmatched_detections[p.second];
		track->update(detection, m_frame_id);
		if (track->state() == s_track_state::tracked) {
			activated_tracks.push_back(track);
		} else {
			tentative_tracks.push_back(track);
		}
	}

	tracking_log::print_matched(unconfirmed_tracks,
		unmatched_detections, matched_pairs, "MATCHED:");

	for (int i : unmatched_track_indexes) {
		unconfirmed_tracks[i]->set_state(s_track_state::removed);
		removed_tracks.push_back(unconfirmed_tracks[i]);
	}

	// Step 4: Init new stracks
	for (int i : unmatched_detection_indexes) {
		unmatched_detections[i]->activate(m_kf, m_frame_id,
			m_use_smooth_feature, m_feature_buffer_size,
			m_smooth_alpha, m_tentative_thresh);
		if (unmatched_detections[i]->state() == s_track_state::tracked) {
			activated_tracks.push_back(unmatched_detections[i]);
		} else {
			new_tracks.push_back(unmatched_detections[i]);
		}
	}

	// Step 5: Update state
	for (s_track_ptr &track : m_lost) {
		if (m_frame_id - track->frame_id() > m_max_lost_age) {
			track->set_state(s_track_state::removed);
			removed_tracks.push_back(track);
		}
	}

	tracking_log::out() << std::endl << "--RESULT--" << std::endl;
	tracking_log::print_tracks(new_tracks, "[new_tracks]");
	tracking_log::print_tracks(activated_tracks, "[activated_tracks]");
	tracking_log::print_tracks(refined_tracks, "[refined_tracks]");
	tracking_log::print_tracks(tentative_tracks, "[tentative_tracks]");
	tracking_log::print_tracks(lost_tracks, "[lost_tracks]");

	for (const s_track_ptr &track : removed_tracks) {
		m_removed.push_back(track);
	}

	while (m_removed.size() > MAX_REMOVED_TRACK_QUEUE_SIZE) {
		m_removed.erase(m_removed.begin());
	}

	m_tracked = choose_tracks(m_tracked,
		[](const s_track_ptr &track)
		{ return track->state() == s_track_state::tracked; });
	m_tracked = joint_tracks(m_tracked, activated_tracks);
	m_tracked = joint_tracks(m_tracked, refined_tracks);
	m_tracked = joint_tracks(m_tracked, tentative_tracks);
	m_tracked = joint_tracks(m_tracked, new_tracks);
	m_lost = sub_tracks(m_lost, m_tracked);
	m_lost.insert(m_lost.end(), lost_tracks.begin(), lost_tracks.end());
	m_lost = sub_tracks(m_lost, m_removed);
	removed_duplicate = remove_duplicate_tracks(m_tracked, m_lost);
	m_available = choose_tracks(m_tracked,
		[](const s_track_ptr &track)
		{ return track->state() == s_track_state::tracked; });

	tracking_log::print_tracks(removed_duplicate,
		"[removed_duplicate]");
	tracking_log::print_tracks(m_tracked, "[m_tracked]");
	tracking_log::print_tracks(m_lost, "[m_lost]");
	tracking_log::print_tracks(m_removed, "[m_removed]");
	tracking_log::print_tracks(m_available, "[m_available]");
	tracking_log::out() << "Update end ----------" << std::endl;
}

dynamic_matrix multi_tracker::embedding_distance(
	const std::vector<s_track_ptr> &tracks,
	const std::vector<s_track_ptr> &detections)
{
	dynamic_matrix ret(tracks.size(), detections.size());
	int row;

	row = 0;
	for (const s_track_ptr &track : tracks) {
		ret.row(row++) = track->embedding_distance(detections);
	}

	return ret;
}

dynamic_matrix multi_tracker::iou_distance(
	const std::vector<s_track_ptr> &tracks,
	const std::vector<s_track_ptr> &detections)
{
	dynamic_matrix ret(tracks.size(), detections.size());
	int row;

	row = 0;
	for (const s_track_ptr &track : tracks) {
		ret.row(row++) = track->iou_distance(detections);
	}

	return ret;
}

void multi_tracker::fuse_motion(dynamic_matrix &distance_matrix,
	const std::vector<s_track_ptr> &tracks,
	const std::vector<s_track_ptr> &detections,
	float fuse_lambda)
{
	float gating_threshold = kalman_filter::chi2inv95[DET_N_DIM];
	int row;

	row = 0;
	for(const s_track_ptr &track : tracks) {
		Eigen::Matrix<float, 1, -1> gating_distance =
			track->gating_distance(detections);
		for (int col = 0; col < gating_distance.cols(); col++) {
			if (fuse_lambda < 1.0f) {
				distance_matrix(row, col) =
					fuse_lambda * distance_matrix(row, col) +
					(1.0f - fuse_lambda) * gating_distance(0, col);
			} else {
				distance_matrix(row, col) = distance_matrix(row, col);
			}

			if (isnan(gating_distance(0, col))
				|| gating_distance(0, col) > gating_threshold) {
				distance_matrix(row, col) = INFINITY_COST;
			}
		}

		row++;
	}
}

void multi_tracker::linear_assignment(const dynamic_matrix &distance_matrix,
	float thresh,
	vector<std::pair<int, int>> &matched_pairs,
	vector<int> &unmatched_track_indexes,
	vector<int> &unmatched_detection_indexes)
{
	matched_pairs.clear();
	unmatched_track_indexes.clear();
	unmatched_detection_indexes.clear();

	do {
		if (distance_matrix.rows() == 0 || distance_matrix.cols() == 0) {
			for (int row = 0; row < distance_matrix.rows(); row++) {
				unmatched_track_indexes.push_back(row);
			}

			for (int col = 0; col < distance_matrix.cols(); col++) {
				unmatched_detection_indexes.push_back(col);
			}

			break;
		}

		std::pair<std::vector<int>, std::vector<int>> matched =
			hungarian_wrap::solve(distance_matrix);
		assert(matched.first.size() == matched.second.size());

		for (int row = 0; row < distance_matrix.rows(); row++) {
			auto lambda = [row](int t) {
				return row == t;
			};
			std::vector<int> &tracks = matched.first;

			if (std::find_if(tracks.begin(), tracks.end(), lambda)
				== tracks.end()) {
				unmatched_track_indexes.push_back(row);
			}
		}

		for (int col = 0; col < distance_matrix.cols(); col++) {
			auto lambda = [col](int d) {
				return col == d;
			};
			std::vector<int> &detections = matched.second;

			if (std::find_if(detections.begin(), detections.end(), lambda)
				== detections.end()) {
				unmatched_detection_indexes.push_back(col);
			}
		}

		for (size_t i = 0; i < matched.first.size(); i++) {
			if (distance_matrix(matched.first[i], matched.second[i]) > thresh) {
				unmatched_track_indexes.push_back(matched.first[i]);
				unmatched_detection_indexes.push_back(matched.second[i]);
			} else {
				matched_pairs.push_back(
					std::make_pair(matched.first[i], matched.second[i]));
			}
		}
	} while (0);

	return;
}

void multi_tracker::match_with_min_cost(distance_type type, float thresh,
	const std::vector<s_track_ptr> &tracks,
	const std::vector<s_track_ptr> &detections,
	std::vector<std::pair<int, int>> &matched_pairs,
	std::vector<int> &unmatched_track_indexes,
	std::vector<int> &unmatched_detection_indexes)
{
	dynamic_matrix distance_matrix;

	tracking_log::print_tracks(tracks, "Matching tracks:");
	tracking_log::print_tracks(detections, "Matching detections:");

	switch (type) {
	case distance_type::embedding:
		distance_matrix = embedding_distance(tracks, detections);
		tracking_log::print_matrix(distance_matrix, "Embedding mat:");
		fuse_motion(distance_matrix, tracks, detections, m_fuse_lambda);
		tracking_log::print_matrix(distance_matrix, "Fuse mat:");
		linear_assignment(distance_matrix, thresh,
			matched_pairs, unmatched_track_indexes,
			unmatched_detection_indexes);
		break;
	case distance_type::iou:
		distance_matrix = iou_distance(tracks, detections);
		tracking_log::print_matrix(distance_matrix, "IOU mat:");
		linear_assignment(distance_matrix, thresh,
			matched_pairs, unmatched_track_indexes,
			unmatched_detection_indexes);
		break;
	default :
		assert(0);
		break;
	}
}

void multi_tracker::match_with_priority(distance_type type, float thresh,
	const std::vector<s_track_ptr> &tracks,
	const std::vector<s_track_ptr> &detections,
	std::vector<std::pair<int, int>> &matched_pairs,
	std::vector<int> &unmatched_track_indexes,
	std::vector<int> &unmatched_detection_indexes)
{
	std::vector<s_track_ptr> handling_tracks;
	std::vector<s_track_ptr> handling_tracks_tmp;
	std::vector<s_track_ptr> handling_detections;
	std::vector<s_track_ptr> handling_detections_tmp;
	std::vector<std::pair<int, int>> handling_matched_pairs;
	std::vector<int> handling_unmatched_track_indexes;
	std::vector<int> handling_unmatched_detection_indexes;

	std::vector<std::pair<s_track_ptr, s_track_ptr>> matched_track_pairs;
	std::vector<s_track_ptr> unmatched_tracks;
	std::vector<s_track_ptr> unmatched_detections;

	matched_pairs.clear();
	unmatched_track_indexes.clear();
	unmatched_detection_indexes.clear();

	for (const s_track_ptr &detection : detections) {
		handling_detections.push_back(detection);
	}

	for (int time = 0; time < m_match_priority_thresh; time++) {
		for (const s_track_ptr &track : tracks) {
			if (m_frame_id - track->frame_id() == time + 1) {
				handling_tracks.push_back(track);
			}
		}

		if (handling_tracks.size() == 0) {
			continue;
		}

		match_with_min_cost(type, thresh, handling_tracks, handling_detections,
			handling_matched_pairs,
			handling_unmatched_track_indexes,
			handling_unmatched_detection_indexes);
		for (std::pair<int, int> &pair : handling_matched_pairs) {
			matched_track_pairs.push_back(
				std::make_pair(handling_tracks[pair.first],
				handling_detections[pair.second]));
		}

		handling_tracks_tmp = handling_tracks;
		handling_tracks.clear();
		for (int i : handling_unmatched_track_indexes) {
			if (m_merge_match_priority) {
				handling_tracks.push_back(handling_tracks_tmp[i]);
			} else {
				unmatched_tracks.push_back(handling_tracks_tmp[i]);
			}
		}

		handling_detections_tmp = handling_detections;
		handling_detections.clear();
		for (int i : handling_unmatched_detection_indexes) {
			handling_detections.push_back(handling_detections_tmp[i]);
		}

		if (handling_detections.size() == 0) {
			break;
		}
	}

	for (const s_track_ptr &track : tracks) {
		if (m_frame_id - track->frame_id() >=
			m_match_priority_thresh + 1) {
			handling_tracks.push_back(track);
		}
	}

	match_with_min_cost(type, thresh, handling_tracks, handling_detections,
		handling_matched_pairs,
		handling_unmatched_track_indexes,
		handling_unmatched_detection_indexes);

	for (std::pair<int, int> &pair : handling_matched_pairs) {
		matched_track_pairs.push_back(
			std::make_pair(handling_tracks[pair.first],
			handling_detections[pair.second]));
	}

	for (int i : handling_unmatched_track_indexes) {
		unmatched_tracks.push_back(handling_tracks[i]);
	}

	for (int i : handling_unmatched_detection_indexes) {
		unmatched_detections.push_back(handling_detections[i]);
	}

	for (std::pair<s_track_ptr, s_track_ptr> &pair : matched_track_pairs) {
		size_t t;
		size_t d;

		for (t = 0; t < tracks.size(); t++) {
			if (pair.first == tracks[t]) {
				break;
			}
		}

		assert(t < tracks.size());
		for (d = 0; d < detections.size(); d++) {
			if (pair.second == detections[d]) {
				break;
			}
		}

		assert(d < detections.size());
		matched_pairs.push_back(std::make_pair<int, int>(t, d));
	}

	for (const s_track_ptr &track : unmatched_tracks) {
		size_t t;

		for (t = 0; t < tracks.size(); t++) {
			if (track == tracks[t]) {
				break;
			}
		}

		assert(t < tracks.size());
		unmatched_track_indexes.push_back(t);
	}

	for (const s_track_ptr &detection : unmatched_detections) {
		size_t d;

		for (d = 0; d < detections.size(); d++) {
			if (detection == detections[d]) {
				break;
			}
		}

		assert(d < detections.size());
		unmatched_detection_indexes.push_back(d);
	}
}

void multi_tracker::remove_unreliable_detections(
	distance_type type,
	float thresh,
	const std::vector<s_track_ptr> &tracks,
	const std::vector<s_track_ptr> &detections,
	std::vector<s_track_ptr> &reliable,
	std::vector<s_track_ptr> &unreliable)
{
	dynamic_matrix distance_matrix;
	dynamic_matrix embedding_matrix;
	std::vector<int> proximity_count(detections.size());

	reliable.clear();
	unreliable.clear();

	switch (type) {
	case distance_type::embedding:
		distance_matrix = embedding_distance(tracks, detections);
		tracking_log::print_matrix(distance_matrix, "Embedding mat:");
		fuse_motion(distance_matrix, tracks, detections, m_fuse_lambda);
		tracking_log::print_matrix(distance_matrix, "Fuse mat:");
		break;
	case distance_type::iou:
		distance_matrix = iou_distance(tracks, detections);
		tracking_log::print_matrix(distance_matrix, "IOU mat:");
		break;
	default :
		assert(0);
		break;
	}

	for (int col = 0; col < distance_matrix.cols(); col++) {
		proximity_count[col] = 0;
		for (int row = 0; row < distance_matrix.rows(); row++) {
			if (distance_matrix(row, col) < thresh) {
				proximity_count[col]++;
			}
		}
	}

	for (size_t i = 0; i < proximity_count.size(); i++) {
		if (proximity_count[i] <= 1) {
			reliable.push_back(detections[i]);
		} else {
			unreliable.push_back(detections[i]);
		}
	}
}

std::vector<s_track_ptr> multi_tracker::joint_tracks(
	const std::vector<s_track_ptr> &a,
	const std::vector<s_track_ptr> &b)
{
	std::vector<s_track_ptr> ret;

	ret = a;

	for (const s_track_ptr &track : b) {
		auto lambda = [&track](s_track_ptr item) {
			return item->track_id() == track->track_id();
		};

		if (std::find_if(a.begin(), a.end(), lambda) == a.end()) {
			ret.push_back(track);
		}
	}

	return ret;
}

std::vector<s_track_ptr> multi_tracker::choose_tracks(
	const std::vector<s_track_ptr> &tracks, bool (*cond)(const s_track_ptr &))
{
	std::vector<s_track_ptr> ret;

	for (const s_track_ptr &track : tracks) {

		if (cond(track)) {
			ret.push_back(track);
		}
	}

	return ret;
}

std::vector<s_track_ptr> multi_tracker::sub_tracks(
	const std::vector<s_track_ptr> &a,
	const std::vector<s_track_ptr> &b)
{
	std::vector<s_track_ptr> ret;

	for (const s_track_ptr &track : a) {
		auto lambda = [&track](s_track_ptr item) {
			return item->track_id() == track->track_id();
		};

		if (std::find_if(b.begin(), b.end(), lambda) == b.end()) {
			ret.push_back(track);
		}
	}

	return ret;
}

std::vector<s_track_ptr> multi_tracker::remove_duplicate_tracks(
	std::vector<s_track_ptr> &a,
	std::vector<s_track_ptr> &b)
{
	dynamic_matrix distance_matrix =
		iou_distance(a, b);
	std::vector<s_track_ptr> dup_a;
	std::vector<s_track_ptr> dup_b;
	std::vector<s_track_ptr> removed;

	for (size_t row = 0; row < a.size(); row++) {
		for (size_t col = 0; col < b.size(); col++) {
			if (distance_matrix(row, col) < m_duplicate_iou_distance) {
				int a_age = a[row]->frame_id() - a[row]->start_frame_id();
				int b_age = b[col]->frame_id() - b[col]->start_frame_id();
				if (a_age > b_age) {
					dup_b.push_back(b[col]);
				} else {
					dup_a.push_back(a[row]);
				}
			}
		}
	}

	for (const s_track_ptr &item : dup_a) {
		a.erase(find(a.begin(), a.end(), item));
		removed.push_back(item);
	}

	for (const s_track_ptr &item : dup_b) {
		b.erase(find(b.begin(), b.end(), item));
		removed.push_back(item);
	}

	return removed;
}
