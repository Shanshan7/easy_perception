/*******************************************************************************
 * hungarian_wrap.cpp
 *
 * History:
 *  2020/12/24  - [Du You] created
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

#include "Hungarian.h"
#include "hungarian_wrap.h"

std::pair<std::vector<int>, std::vector<int>> hungarian_wrap::solve(
	const Eigen::Matrix<float, -1, -1, Eigen::RowMajor> &cost_matrix)
{
	std::vector<std::vector<double>> matrix(cost_matrix.rows());
	std::vector<int> assignment;
	std::pair<std::vector<int>, std::vector<int>> solution;

	for (int row = 0; row < cost_matrix.rows(); row++) {
		for (int col = 0; col < cost_matrix.cols(); col++) {
			matrix[row].push_back(cost_matrix(row, col));
		}
	}

	HungarianAlgorithm().Solve(matrix, assignment);

	for (size_t row = 0; row < assignment.size(); row++) {
		if (assignment[row] != -1) {
			solution.first.push_back(row);
			solution.second.push_back(assignment[row]);
		}
	}

	return solution;
}
