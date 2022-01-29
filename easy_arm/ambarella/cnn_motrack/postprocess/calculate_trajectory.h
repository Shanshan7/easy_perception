#ifndef _CALCULATE_TRAJECTORY_H_
#define _CALCULATE_TRAJECTORY_H_

#include "../fairmot/mot.h"
#include "../common/data_struct.h"


int calculate_tracking_trajectory(mot_result_t *mot_result, std::map<int, TrajectoryParams> &track_idx_map);
int calculate_tracking_trajectory(std::vector<DetectBox>& boxes, int frame_id, int data_height);


#endif // _CALCULATE_TRAJECTORY_H_