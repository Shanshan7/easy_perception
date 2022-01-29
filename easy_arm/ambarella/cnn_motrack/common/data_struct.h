#ifndef _DATA_STRUCT_H_
#define _DATA_STRUCT_H_

#include <config.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <wchar.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <vector>
#include <map>
#include <glog/logging.h>
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
	std::vector<float> pedestrian_x_end;
	std::vector<float> pedestrian_y_end;
	std::vector<cv::Point2f> trajectory_position;
};

typedef struct DetectBox {
    DetectBox(float x1=0, float y1=0, float x2=0, float y2=0, 
            float confidence=0, float classID=-1, float trackID=-1) {
        this->x1 = x1;
        this->y1 = y1;
        this->x2 = x2;
        this->y2 = y2;
        this->confidence = confidence;
        this->classID = classID;
        this->trackID = trackID;
    }
    float x1, y1, x2, y2;
    float confidence;
    float classID;
    float trackID;
} DetectBox;

struct GlobalControlParam {
	// cmd line param
	// uint8_t channel_id;
	// uint8_t stream_id;
	// uint32_t state_buf_num;
	// float overlay_text_width_ratio;
	// float overlay_x_offset;
	// uint16_t overlay_highlight_sec;
	// uint16_t overlay_clear_sec;
	// uint32_t verbose; /* network print time */
	// uint32_t debug_en; /* 0: disable; 1: time measure,2: log; 3: run once & save picture */
	// uint32_t abort_if_preempted;
	// ea_img_resource_t *img_resource;

	// run time control
	// state_buffer_param_t ssd_result_buf;
	// pthread_mutex_t access_buffer_mutex;
	// sem_t sem_readable_buf;
	// pthread_mutex_t vp_access_lock;
	int run_flag;

	int current_frame;
	
	// results
	std::map<int, TrajectoryParams> track_idx_map;

};


#endif // _DATA_STRUCT_H_