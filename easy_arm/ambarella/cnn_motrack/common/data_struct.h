#ifndef _DATA_STRUCT_H_
#define _DATA_STRUCT_H_

#include <config.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <vector>
#include <map>
#include <opencv2/core.hpp>

#include <eazyai.h>

#define MOT_MAX_TRACK_NUM	        100



// track struct
struct TrackAttribute {
	float fdet_conf;             // detect confidence
	int ncls;                    // class
	int ntrack_id;               // track id
	float fobject_loc[4];        // object location: Image Coordinate, x1, y1, x2, y2
};

struct TrackOutPut {
	TrackAttribute track_attri[MOT_MAX_TRACK_NUM];
	int nframe_index;
	int nvalid_track_count;
};

struct TrajectoryParams {
	int latest_frame_id=-1;      // the latest frame the target has captured recently
	int draw_flag;
    int npedestrian_direction;
	float mean_velocity;
	std::vector<float> velocity_vector;
	std::vector<float> pedestrian_x_start;
	std::vector<float> pedestrian_y_start;
	std::vector<cv::Point2f> trajectory_position;
};


#endif // _DATA_STRUCT_H_