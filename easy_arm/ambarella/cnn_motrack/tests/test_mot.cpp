#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <signal.h>

#include "image_acquisition/save_img_process.h"

static SaveImageProcess save_image_process;
static save_data_t save_data;


static void *run_image_pthread(void *thread_params)
{
    int rval = 0;
    save_data.canvas_id = 0;

    do {
        RVAL_OK(save_image_process.cv_env_init(&save_data));
    } while (0);

    do {
        save_image_process.init_save_dir();
        snprintf(save_data.output_dir, sizeof(save_data.output_dir), "%s", \
                save_image_process.save_path.str().c_str());
        EA_LOG_NOTICE("SAVE_PATH: %s\n", save_image_process.save_path.str().c_str());
        

        do {
            RVAL_OK(save_image_process.cv_img_acquisition(&save_data));

            if (save_data.sig_flag) {
                break;
            }

            EA_LOG_NOTICE("IMAGE_FRAME_NUM: %d\n", save_image_process.image_frame_number);
        } while(save_image_process.image_frame_number % 1500 != 0);

        if (save_data.sig_flag) {
            break;
        }
    } while(1);

    save_image_process.cv_env_deinit(&save_data);
	EA_LOG_DEBUG("run_image_pthread quit\n");

	return NULL;
}

static double disk_rate_calculate() {
    double disk_rate=0.0;
    struct statfs sf;
    if(0 == statfs(save_image_process.save_dir.c_str(), &sf)){
            disk_rate=((sf.f_blocks-sf.f_bfree)*100.0/sf.f_blocks);
    }else{
            std::cout << "error:" << std::endl;
    }
    std::cout << "disk rate: "<< disk_rate << std::endl;

    return disk_rate;
}

static void sig_stop(int a)
{
	(void)a;
	save_data.sig_flag = 1;
}

int main() {
    int rval = 0;

    // process signals to control program operation
    signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);

    // void *status_m;
    // std::cout << "status_m addr is " << &status_m << std::endl;

    pthread_t image_pthread_id = 0;
    rval = pthread_create(&image_pthread_id, NULL, run_image_pthread, NULL);
    if (rval < 0) {
        std::cout << "[ERROR]: create pthread fail! \n";
    }

    double disk_rate = disk_rate_calculate();

    do {
        std::cout << "Determine whether there is enough memory" << std::endl;
        if (disk_rate > 97.0) {
            std::cout << "quit!!!" << std::endl;
            break;
        }
        if (save_data.sig_flag) {
			break;
		}
        disk_rate = disk_rate_calculate();
        sleep(60);
    } while(1);

    EA_LOG_NOTICE("Save image success!!!");

    return rval;
}
