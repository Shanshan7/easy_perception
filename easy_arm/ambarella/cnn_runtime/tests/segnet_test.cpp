#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include "utility/utils.h"
#include "cnn_runtime/segment/segnet.h"

const static std::string model_path = "./segnet.bin";
const static std::string input_name = "seg_input";
const static std::string output_name = "seg_output";

static void image_dir_infer(const std::string &image_dir){
    const std::string save_result_dir = "./seg_result/";
    unsigned long time_start, time_end;
    std::vector<std::string> images;
    cv::Mat src_image;
    cv::Mat result_mat;
    SegNet seg_process;
    if(seg_process.init(model_path, input_name, output_name) < 0)
    {
        std::cout << "SegNet init fail!" << std::endl;
        return;
    }
    ListImages(image_dir, images);
    std::cout << "total Test images : " << images.size() << std::endl;
    for (size_t index = 0; index < images.size(); index++) {
        std::stringstream save_path;
		std::stringstream temp_str;
        temp_str << image_dir << images[index];
        size_t str_index = images[index].find_first_of('.', 0);
        std::string image_name = images[index].substr(0, str_index);
		std::cout << temp_str.str() << std::endl;
		src_image = cv::imread(temp_str.str());
        time_start = get_current_time();
        seg_process.run(src_image);
        time_end = get_current_time();
        std::cout << "seg cost time: " <<  (time_end - time_start) / 1000.0  << "ms" << std::endl;

        save_path << save_result_dir << image_name << ".png";
        cv::imwrite(save_path.str(), result_mat);
    }
}


void image_txt_infer(const std::string &image_dir, const std::string &image_txt_path){
    const std::string save_result_dir = "./seg_result/";
    unsigned long time_start, time_end;
    std::ifstream read_txt;
    std::string line_data;
    cv::Mat src_image;
    cv::Mat result_mat;
    SegNet seg_process;
    if(seg_process.init(model_path, input_name, output_name) < 0)
    {
        std::cout << "SegNet init fail!" << std::endl;
        return;
    }

    read_txt.open(image_txt_path.data());
    if(!read_txt.is_open()){
        std::cout << image_txt_path << " not exits" << std::endl;
        return;
    }
    
    while(std::getline(read_txt, line_data)){
        if(line_data.empty()){
            continue;
        }
        size_t str_index = line_data.find_first_of(' ', 0);
        std::string image_name_post = line_data.substr(0, str_index);
        str_index = image_name_post.find_first_of('.', 0);
        std::string image_name = line_data.substr(0, str_index);
        std::stringstream save_path;
        std::stringstream image_path;
        image_path << image_dir << image_name_post;
        std::cout << image_path.str() << std::endl;
        src_image = cv::imread(image_path.str());
        time_start = get_current_time();
        seg_process.run(src_image);
        time_end = get_current_time();
        std::cout << "seg cost time: " <<  (time_end - time_start) / 1000.0  << "ms" << std::endl;

        save_path << save_result_dir << image_name << ".png";
        cv::imwrite(save_path.str(), result_mat);
    }
    read_txt.close();
}

int main()
{
    std::cout << "start..." << std::endl;
    const std::string image_dir = "./images/";
    const std::string image_txt_path = "";
    image_dir_infer(image_dir);
    // image_txt_infer(image_dir, image_txt_path);
    std::cout << "End of game!!!" << std::endl;
    return 0;
}
