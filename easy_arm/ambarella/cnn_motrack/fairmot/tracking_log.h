/*******************************************************************************
 * tracking_log.h
 *
 * History:
 *  2021/01/04 - [Du You] create file
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

#ifndef TRACKING_LOG_H_
#define TRACKING_LOG_H_

#include <iostream>
#include <iomanip>

#include "kalman_filter.h"
#include "s_track.h"

class tracking_log
{
public:
	static std::ostream &out()
	{
		if (m_enabled) {
			return m_out;
		} else {
			return m_dummy;
		}
	}

	static void enable(bool enabled)
	{
		m_enabled = enabled;
	}

	static void print_matrix(const dynamic_matrix &matrix, const std::string &mes)
	{
		tracking_log::out() << mes << std::endl;
		if (matrix.rows() > 0 && matrix.cols() > 0) {
			for (int row = 0; row < matrix.rows(); row++) {
				for (int col = 0; col < matrix.cols(); col++) {
					tracking_log::out() << std::fixed << std::setprecision(6)
						<< std::setw(8) << std::setfill(' ')
						<< matrix(row, col) << "\t";
				}

				tracking_log::out() << std::endl;
			}
		}
	}

	static void print_reid_features(const reid_features &features,
		const std::string &mes)
	{
		tracking_log::out() << mes << std::endl;

		for (int row = 0; row < features.rows(); row++) {
			for (int col = 0; col < features.cols(); col++) {
				tracking_log::out() << std::fixed << std::setprecision(6)
					<< std::setw(8) << std::setfill(' ')
					<< features(row, col) << "\t";
			}

			tracking_log::out() << std::endl;
		}
	}

	static void print_tracks(const std::vector<s_track_ptr> tracks,
		const std::string &mes)
	{
		tracking_log::out() << mes << std::endl;
		if (tracks.size() > 0) {
			tracking_log::out() << "track_id\tframe_id\tx\t\ttentative" << std::endl;
			for (const s_track_ptr &track : tracks) {
				tracking_log::out() << track->track_id() << "\t\t"
					<< track->frame_id() << "/" << track->start_frame_id()
					<< "\t\t"
					<< std::fixed << std::setprecision(6)
					<< std::setw(8) << std::setfill(' ')
					<< track->to_normalized_det_tlwh()(0) << "\t"
					<< track->tentative_id()
					<< std::endl;
			}
		}
	}

	static void print_matched(const std::vector<s_track_ptr> &tracks,
		const std::vector<s_track_ptr> &detections,
		const std::vector<std::pair<int, int>> &matched_pairs,
		const std::string &mes)
	{
		tracking_log::out() << mes << std::endl;
		if (matched_pairs.size() > 0) {
			tracking_log::out() << "track_id\tx\t\ttentative" << std::endl;
			for (const std::pair<int, int> &p : matched_pairs) {
				tracking_log::out() << tracks[p.first]->track_id() << "\t\t"
					<< std::fixed << std::setprecision(6)
					<< std::setw(8) << std::setfill(' ')
					<< detections[p.second]->to_normalized_det_tlwh()(0) << "\t"
					<< tracks[p.first]->tentative_id()
					<< std::endl;
			}
		}
	}

private:
	static bool m_enabled;
	static std::ostream &m_out;
	static std::ostream m_dummy;
	tracking_log() {}
};

#endif // TRACKING_LOG_H_
