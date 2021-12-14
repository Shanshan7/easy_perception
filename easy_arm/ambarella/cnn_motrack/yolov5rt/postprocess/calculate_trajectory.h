#ifndef CALCULATE_TRAJECTORY_H
#define CALCULATE_TRAJECTORY_H

#include "datatype.h"
#include <glog/logging.h>
#include <map>


void calculate_tracking_trajectory(std::vector<DetectBox>& boxes, std::map<int, TrajectoryParams> &track_idx_map, int frame_id);

#endif // CALCULATE_TRAJECTORY_H