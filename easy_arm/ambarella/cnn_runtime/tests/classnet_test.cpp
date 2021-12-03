#include <iostream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include "utility/utils.h"
#include "cnn_runtime/classify/classnet.h"

const static std::string model_path = "./classnet.bin";
const static std::string input_name = "cls_input";
const static std::string output_name = "cls_output";
const static int class_num = 100;

static void image_dir_infer(const std::string &image_dir){
    unsigned long time_start, time_end;
    std::vector<std::string> images;
    std::ofstream save_result;
    int class_idx = -1;
    cv::Mat src_img;
    ClassNet classnet_process;
    if(classnet_process.init(model_path, input_name, output_name, class_num) < 0)
    {
        std::cout << "ClassNet init fail!" << std::endl;
        return;
    }
    ListImages(image_dir, images);
    std::cout << "total Test images : " << images.size() << std::endl;
    save_result.open("./cls_result.txt");
    for (size_t index = 0; index < images.size(); index++) {
		std::stringstream temp_str;
        temp_str << image_dir << images[index];
		std::cout << temp_str.str() << std::endl;
		src_img = cv::imread(temp_str.str());
        time_start = get_current_time();
        class_idx = classnet_process.run(src_img);
        time_end = get_current_time();
        std::cout << "classnet cost time: " <<  (time_end - time_start)/1000.0  << "ms" << std::endl;

        save_result << images[index] << " " << class_idx << "\n";
    }
    save_result.close();
}

static void image_txt_infer(const std::string &image_dir, const std::string &image_txt_path){
    unsigned long time_start, time_end;
    std::ofstream save_result;
    std::ifstream read_txt;
    int class_idx = -1;
    std::string line_data;
    cv::Mat src_img;
    ClassNet classnet_process;
    if(classnet_process.init(model_path, input_name, output_name, class_num) < 0)
    {
        std::cout << "ClassNet init fail!" << std::endl;
        return;
    }

    read_txt.open(image_txt_path.data());
    if(!read_txt.is_open()){
        std::cout << image_txt_path << " not exits" << std::endl;
        return;
    }
    
    save_result.open("./cls_result.txt");
    while(std::getline(read_txt, line_data)){
        if(line_data.empty()){
            continue;
        }
        size_t index = line_data.find_first_of(' ', 0);
        std::string image_name = line_data.substr(0, index);
        std::stringstream image_path;
        image_path << image_dir << image_name;
        std::cout << image_path.str() << std::endl;
        src_img = cv::imread(image_path.str());
        time_start = get_current_time();
        class_idx = classnet_process.run(src_img);
        time_end = get_current_time();
        std::cout << "classnet cost time: " <<  (time_end - time_start)/1000.0  << "ms" << std::endl;

        save_result << image_name << " " << class_idx << "\n";
    }
    read_txt.close();
    save_result.close();
}

int main()
{
    std::cout << "start..." << std::endl;
    std::string image_dir = "./test_images/";
    image_dir_infer(image_dir);
    std::cout << "End of game!!!" << std::endl;
    return 0;
}
