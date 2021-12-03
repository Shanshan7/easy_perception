#ifndef _SAVE_IMAGE_PROCESS_H_
#define _SAVE_IMAGE_PROCESS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <eazyai.h>

#define MAX_PATH_STRLEN			    256
#define TRACK_SAVE_PATH             "/sdcard/fairmot/"
EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_VERBOSE);

typedef struct save_data_s {
	int canvas_id;

	int sig_flag;
    char output_dir[MAX_PATH_STRLEN + 1];

	ea_img_resource_t *img_resource;
	ea_display_t *display;

} save_data_t;

class SaveImageProcess
{
public:
    SaveImageProcess();
    ~SaveImageProcess();

    int init_save_dir();
    int cv_env_init(save_data_t *save_data);

    int cv_img_acquisition(save_data_t *save_data);
    int cv_video_acquisition();

    void cv_env_deinit(save_data_t *save_data);

    std::string save_dir;
    std::stringstream save_path;
    unsigned long long int image_frame_number; // *frame number can stack binary code*

private:
    pthread_t image_pthread_id;
    pthread_t video_pthread_id;
};

#endif // _SAVE_IMAGE_PROCESS_H_