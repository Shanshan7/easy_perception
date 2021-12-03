#include "save_data_process.h"
#include "utility/utils.h"
#include <iostream>
#include <fstream>

static struct SaveImageBuffer image_buffer;
// static struct SaveImageBuffer video_buffer;
static struct SaveTofBuffer tof_buffer; 

static std::string current_save_dir = "./";

volatile int save_run = 0;
volatile int video_save_run = 0; 

// static bool saveMapBin(const std::string& filePath, const cv::Mat& map)
// {
// 	if (map.empty())
// 		return false;
 
// 	const char* filenamechar = filePath.c_str();
// 	FILE* fpw = fopen(filenamechar, "wb");//如果没有则创建，如果存在则从头开始写
// 	if (fpw == NULL)
// 	{
// 		//不可取fclose(fpw);
// 		return false;
// 	}
 
// 	int chan = map.channels();//用1个字节存,通道
// 	int type = map.type();//用2个字节存,类型,eg.CV_16SC2,CV_16UC1,CV_8UC1...
// 	int rows = map.rows;//用4个字节存,行数
// 	int cols = map.cols;//用4个字节存,列数
 
// 	fwrite(&chan, sizeof(char), 1, fpw);
// 	fwrite(&type, sizeof(char), 2, fpw);
// 	fwrite(&rows, sizeof(char), 4, fpw);
// 	fwrite(&cols, sizeof(char), 4, fpw);
 
// 	if (chan == 3)
// 	{
// 		if (type == CV_8UC3)//8U代表8位无符号整形,C3代表三通道
// 		{
// 			uchar* pData = (uchar*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fwrite(&pData[i * 3], sizeof(uchar), 1, fpw);
// 				fwrite(&pData[i * 3 + 1], sizeof(uchar), 1, fpw);
// 				fwrite(&pData[i * 3 + 2], sizeof(uchar), 1, fpw);
// 			}
// 		}
// 		else
// 		{
// 			fclose(fpw);
// 			return false;
// 		}
// 	}
// 	else if (chan == 2)
// 	{
// 		if (type == CV_8UC2)
// 		{
// 			uchar* pData = (uchar*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fwrite(&pData[i * 2], sizeof(uchar), 1, fpw);
// 				fwrite(&pData[i * 2 + 1], sizeof(uchar), 1, fpw);
// 			}
// 		}
// 		else if (type == CV_16SC2)//16S代表16位有符号整形,C2代表双通道
// 		{
// 			short* pData = (short*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fwrite(&pData[i * 2], sizeof(short), 1, fpw);
// 				fwrite(&pData[i * 2 + 1], sizeof(short), 1, fpw);
// 			}
// 		}
// 		else
// 		{
// 			fclose(fpw);
// 			return false;
// 		}
// 	}
// 	else if (chan == 1)
// 	{
// 		if (type == CV_8UC1)
// 		{
// 			uchar* pData = (uchar*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fwrite(&pData[i], sizeof(uchar), 1, fpw);
// 			}
// 		}
// 		else if (type == CV_16UC1)//16U代表16位无符号整形,C1代表单通道
// 		{
// 			ushort* pData = (ushort*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fwrite(&pData[i], sizeof(ushort), 1, fpw);
// 			}
// 		}
// 		else
// 		{
// 			fclose(fpw);
// 			return false;
// 		}
// 	}
// 	else
// 	{
// 		fclose(fpw);
// 		return false;
// 	}
 
// 	fclose(fpw);
// 	return true;
// }
 
// static bool loadMapBin(const std::string& filePath, cv::Mat& map)
// {
// 	const char* filenamechar = filePath.c_str();
// 	FILE* fpr = fopen(filenamechar, "rb");
// 	if (fpr == NULL)
// 	{
// 		//不可取fclose(fpr);
// 		return false;
// 	}
 
// 	int chan = 0;
// 	int type = 0;
// 	int rows = 0;
// 	int cols = 0;
 
// 	fread(&chan, sizeof(char), 1, fpr);//1个字节存,通道
// 	fread(&type, sizeof(char), 2, fpr);//2个字节存,类型,eg.CV_16SC2,CV_16UC1,CV_8UC1...
// 	fread(&rows, sizeof(char), 4, fpr);//4个字节存,行数
// 	fread(&cols, sizeof(char), 4, fpr);//4个字节存,列数
 
// 	map = cv::Mat::zeros(rows, cols, type);
 
// 	if (chan == 3)
// 	{
// 		if (type == CV_8UC3)//8U代表8位无符号整形,C3代表三通道
// 		{
// 			uchar* pData = (uchar*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fread(&pData[i * 3], sizeof(uchar), 1, fpr);
// 				fread(&pData[i * 3 + 1], sizeof(uchar), 1, fpr);
// 				fread(&pData[i * 3 + 2], sizeof(uchar), 1, fpr);
// 			}
// 		}
// 		else
// 		{
// 			fclose(fpr);
// 			return false;
// 		}
// 	}
// 	else if (chan == 2)
// 	{
// 		if (type == CV_8UC2)
// 		{
// 			uchar* pData = (uchar*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fread(&pData[i * 2], sizeof(uchar), 1, fpr);
// 				fread(&pData[i * 2 + 1], sizeof(uchar), 1, fpr);
// 			}
// 		}
// 		else if (type == CV_16SC2)//16S代表16位有符号整形,C2代表双通道
// 		{
// 			short* pData = (short*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fread(&pData[i * 2], sizeof(short), 1, fpr);
// 				fread(&pData[i * 2 + 1], sizeof(short), 1, fpr);
// 			}
// 		}
// 		else
// 		{
// 			fclose(fpr);
// 			return false;
// 		}
// 	}
// 	else if (chan == 1)
// 	{
// 		if (type == CV_8UC1)
// 		{
// 			uchar* pData = (uchar*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fread(&pData[i], sizeof(uchar), 1, fpr);
// 			}
// 		}
// 		else if (type == CV_16UC1)//16U代表16位无符号整形,C1代表单通道
// 		{
// 			ushort* pData = (ushort*)map.data;
// 			for (int i = 0; i < rows * cols; i++)
// 			{
// 				fread(&pData[i], sizeof(ushort), 1, fpr);
// 			}
// 		}
// 		else
// 		{
// 			fclose(fpr);
// 			return false;
// 		}
// 	}
// 	else
// 	{
// 		fclose(fpr);
// 		return false;
// 	}
 
// 	fclose(fpr);
// 	return true;
// }

// static void *save_video_pthread(void* save_data)
// {
// 	int rval = 0;
// 	bool first_save = true;
// 	unsigned long long int frame_number = 0;
// 	cv::Mat image_mat;
// 	cv::VideoWriter output_video;
// 	struct timeval tv;  
//     char time_str[64]; 
// 	while (video_save_run > 0) 
// 	{
//         pthread_mutex_lock(&video_buffer.lock);  
//         if (video_buffer.writepos == video_buffer.readpos)  
//         {  
//             pthread_cond_wait(&video_buffer.notempty, &video_buffer.lock);  
//         }
//         image_mat = video_buffer.buffer[video_buffer.readpos];
//         video_buffer.readpos++;  
//         if (video_buffer.readpos >= SAVE_IMAGE_BUFFER_SIZE)  
//             video_buffer.readpos = 0; 
//         pthread_cond_signal(&video_buffer.notfull);  
//         pthread_mutex_unlock(&video_buffer.lock);
//         if(first_save)
//         {
//             if (output_video.isOpened())
//             {
//                 output_video.release();
//             }
//             std::stringstream filename;
//             gettimeofday(&tv, NULL);  
//             strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d_%H:%M:%S", localtime(&tv.tv_sec)); 
//             filename << "./result_video/" << time_str << ".avi";
//             if(output_video.open(filename.str(), cv::VideoWriter::fourcc('X','V','I','D'), 25, \
//                 cv::Size(image_mat.cols, image_mat.rows)))
//             {
//                 output_video.write(image_mat);
//                 first_save = false;
//             }
//         }
//         else
//         {
//             if (output_video.isOpened())
//             {
//                 output_video.write(image_mat);
//             }
//         }
// 		// std::cout << "save video runing" << std::endl;
// 	}
// 	if(output_video.isOpened())
//     {
//         output_video.release();
//     }
//     LOG(WARNING) << "stop save video pthread.";
// 	return NULL;
// }

static void *save_image_pthread(void* save_data)
{
    unsigned long long int frame_number = 0;
    prctl(PR_SET_NAME, "save_image_pthread");
    while(save_run) {
        pthread_mutex_lock(&image_buffer.lock);  
        if (image_buffer.writepos == image_buffer.readpos)  
        {  
            pthread_cond_wait(&image_buffer.notempty, &image_buffer.lock);  
        }
        cv::Mat src_image = image_buffer.buffer[image_buffer.readpos];
        std::stringstream filename_image;
        filename_image << current_save_dir << "image_" << frame_number << ".jpg";
        if(!src_image.empty())
        {
            cv::imwrite(filename_image.str(), src_image);
        }
        else
        {
            LOG(ERROR) << "save src image empty: " << filename_image.str();
        }
        frame_number++;
        image_buffer.readpos++;  
        if (image_buffer.readpos >= SAVE_IMAGE_BUFFER_SIZE)  
            image_buffer.readpos = 0; 
        pthread_cond_signal(&image_buffer.notfull);  
        pthread_mutex_unlock(&image_buffer.lock);
	}
	save_run = 0;
	LOG(WARNING) << "save image thread quit.";
    return NULL;
}

static void *save_tof_pthread(void* save_data)
{
    unsigned long long int frame_number = 0;
    prctl(PR_SET_NAME, "save_tof_pthread");
    while(save_run) {
		pthread_mutex_lock(&tof_buffer.lock);  
        if (tof_buffer.writepos == tof_buffer.readpos)  
        {  
            pthread_cond_wait(&tof_buffer.notempty, &tof_buffer.lock);  
        }
        cv::Mat depth_map = tof_buffer.buffer[tof_buffer.readpos];
        std::stringstream filename_tof;
        filename_tof << current_save_dir << "tof_" << frame_number << ".jpg";
        if(!depth_map.empty())
        {
            cv::imwrite(filename_tof.str(), depth_map);
        }
        else
        {
            LOG(ERROR) << "save depth map empty: " << filename_tof.str();
        }
        frame_number++;
        tof_buffer.readpos++;  
        if (tof_buffer.readpos >= SAVE_TOF_BUFFER_SIZE)  
            tof_buffer.readpos = 0; 
        pthread_cond_signal(&tof_buffer.notfull);  
        pthread_mutex_unlock(&tof_buffer.lock);
	}
	save_run = 0;
	LOG(WARNING) << "save tof thread quit.";
    return NULL;
}

SaveDataProcess::SaveDataProcess()
{
    image_pthread_id = 0;
    tof_pthread_id = 0;
    video_pthread_id = 0;
    save_run = 0;
    video_save_run = 0;

    pthread_mutex_init(&image_buffer.lock, NULL);  
    pthread_cond_init(&image_buffer.notempty, NULL);  
    pthread_cond_init(&image_buffer.notfull, NULL);  
    image_buffer.readpos = 0;  
    image_buffer.writepos = 0;

    pthread_mutex_init(&tof_buffer.lock, NULL);  
    pthread_cond_init(&tof_buffer.notempty, NULL);  
    pthread_cond_init(&tof_buffer.notfull, NULL);  
    tof_buffer.readpos = 0;  
    tof_buffer.writepos = 0;

    save_dir = "/data/save_data/";
    save_index = 0;

    tof_frame_number = 0;
    image_frame_number = 0;
}

SaveDataProcess::~SaveDataProcess()
{
#ifndef ONLY_SAVE_DATA
    if(save_run > 0)
	{
		stop();
	}
#endif
    pthread_mutex_destroy(&image_buffer.lock);
    pthread_cond_destroy(&image_buffer.notempty);
    pthread_cond_destroy(&image_buffer.notfull);

    pthread_mutex_destroy(&tof_buffer.lock);
    pthread_cond_destroy(&tof_buffer.notempty);
    pthread_cond_destroy(&tof_buffer.notfull);

    LOG(WARNING) << "~SaveDataProcess()";
}

int SaveDataProcess::init_data()
{
    std::vector<std::string> dir_list;
    ListPath(save_dir, dir_list);
    for (size_t index = 0; index < dir_list.size(); index++) 
    {
        size_t pos = dir_list[index].find_last_of('_');
        LOG(WARNING) << dir_list[index] << " " << pos;
        if(pos >= 0)
        {
            std::string temp = dir_list[index].substr(pos + 1);
            int temp_index = atoi(temp.c_str());
            if(temp_index > save_index)
            {
                save_index = temp_index;
            }
        }
    }
    save_index++;
    tof_frame_number = 0;
    image_frame_number = 0;
    LOG(WARNING) << save_index;
    return 0;
}

int SaveDataProcess::init_save_dir()
{
    struct timeval tv;  
    char time_str[64];
	std::stringstream save_path;
    std::string command;
    save_run = 1;
    gettimeofday(&tv, NULL); 
	strftime(time_str, sizeof(time_str)-1, "%Y_%m_%d_%H_%M_%S", localtime(&tv.tv_sec)); 
	save_path << save_dir << time_str << "_" << save_index << "/";
    current_save_dir = save_path.str();
    command = "mkdir -p " + current_save_dir;
    system(command.c_str());
    // if(!system(command.c_str()))
    // {
    //     save_run = 0;
    //     LOG(ERROR) << "mkdir dir fail:" << command;
    //     return -1;
    // }
    // if (0 != access(save_path.str().c_str(), 0))
    // {
    //     // if this folder not exist, create a new one.
    //     mkdir(save_path.str().c_str());   // 返回 0 表示创建成功，-1 表示失败
    // }
    // else
    // {
    //     save_run = 0;
    //     LOG(ERROR) << "mkdir dir fail:" << command;
    //     return -1;
    // }
    return 0;
}

int SaveDataProcess::start()
{
    int ret = 0;
    ret = init_save_dir();
    image_buffer.readpos = 0;  
    image_buffer.writepos = 0;

    tof_buffer.readpos = 0;  
    tof_buffer.writepos = 0;

    ret = pthread_create(&image_pthread_id, NULL, save_image_pthread, NULL);
    if(ret < 0)
    {
        save_run = 0;
        LOG(ERROR) << "save image pthread fail!";
    }
    else
    {
        LOG(WARNING) << "start image tof pthread:" << image_pthread_id;
        ret = pthread_create(&tof_pthread_id, NULL, save_tof_pthread, NULL);
        if(ret < 0)
        {
            save_run = 0;
            LOG(ERROR) << "save tof pthread fail!";
        }
        LOG(WARNING) << "start save tof pthread:" << tof_pthread_id;
    }
    LOG(INFO) << "save pthread start success!";
	return ret;
}

int SaveDataProcess::stop()
{
    int ret = 0;
	save_run = 0;

    // LOG(WARNING) << "stop save data";

	if (image_pthread_id > 0) {
        pthread_cond_signal(&image_buffer.notempty);  
        pthread_mutex_unlock(&image_buffer.lock);
		pthread_join(image_pthread_id, NULL);
        image_pthread_id = 0;
	}
    if (tof_pthread_id > 0) {
        pthread_cond_signal(&tof_buffer.notempty);  
        pthread_mutex_unlock(&tof_buffer.lock);
		pthread_join(tof_pthread_id, NULL);
        tof_pthread_id = 0;
	}
	LOG(WARNING) << "stop save data success";
	return ret;
}

int SaveDataProcess::video_start()
{
    return 0;
}

int SaveDataProcess::video_stop()
{
    return 0;
}

void SaveDataProcess::put_image_data(cv::Mat &src_image)
{
    if (image_pthread_id > 0) 
    {
        if(!src_image.empty())
        {
            pthread_mutex_lock(&image_buffer.lock);  
            if ((image_buffer.writepos + 1) % SAVE_IMAGE_BUFFER_SIZE == image_buffer.readpos)  
            {  
                pthread_cond_wait(&image_buffer.notfull, &image_buffer.lock);  
            }
            
            image_buffer.buffer[image_buffer.writepos] = src_image.clone();
            
            image_buffer.writepos++;  
            if (image_buffer.writepos >= SAVE_IMAGE_BUFFER_SIZE)  
                image_buffer.writepos = 0;  
            pthread_cond_signal(&image_buffer.notempty);  
            pthread_mutex_unlock(&image_buffer.lock);
        }
        else
        {
            LOG(ERROR) << "put src image is empty!";
        }
    } 
}

void SaveDataProcess::put_tof_data(cv::Mat &depth_map)
{
    if(tof_pthread_id > 0)
    {
        if(!depth_map.empty())
        {
            pthread_mutex_lock(&tof_buffer.lock);  
            if ((tof_buffer.writepos + 1) % SAVE_TOF_BUFFER_SIZE == tof_buffer.readpos)  
            {  
                pthread_cond_wait(&tof_buffer.notfull, &tof_buffer.lock);  
            }
            tof_buffer.buffer[tof_buffer.writepos] = depth_map.clone();
            tof_buffer.writepos++;  
            if (tof_buffer.writepos >= SAVE_TOF_BUFFER_SIZE)  
                tof_buffer.writepos = 0;  
            pthread_cond_signal(&tof_buffer.notempty);  
            pthread_mutex_unlock(&tof_buffer.lock);
        }
        else
        {
            LOG(ERROR) << "put depth map is empty!";
        }
    }  
}

void SaveDataProcess::save_image(cv::Mat &src_image)
{
    std::stringstream filename_image;
    filename_image << current_save_dir << "image_" << image_frame_number << ".jpg";
    if(!src_image.empty())
    {
        // std::ofstream outF(filename_image.str(), std::ios::binary);
        // outF.write(reinterpret_cast<char*>(src_image.data), IMAGE_WIDTH * IMAGE_HEIGHT * 3 * sizeof(uchar));
        // outF.close();
        cv::imwrite(filename_image.str(), src_image);
    }
    else
    {
        LOG(ERROR) << "save src image empty: " << filename_image.str();
    }
    image_frame_number++;
}

void SaveDataProcess::save_tof(cv::Mat &depth_map)
{
    std::stringstream filename_tof;
    filename_tof << current_save_dir << "tof_" << tof_frame_number << ".bin";
    if(!depth_map.empty())
    {
        std::ofstream outF(filename_tof.str(), std::ios::binary);
        outF.write(reinterpret_cast<char*>(depth_map.data), DEPTH_WIDTH * DEPTH_HEIGTH * sizeof(uchar));
        outF.close();
        // cv::imwrite(filename_tof.str(), depth_map);
    }
    else
    {
        LOG(ERROR) << "save depth map empty: " << filename_tof.str();
    }
    tof_frame_number++;
}
