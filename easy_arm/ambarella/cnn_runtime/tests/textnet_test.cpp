#include <iostream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include "utility/utils.h"
#include "cnn_runtime/rec_text/textnet.h"

const static std::string model_path = "./TextNet.bin";
const static std::string input_name = "text_input";
const static std::string output_name = "text_output";


static void image_dir_infer(const std::string &image_dir){
    unsigned long time_start, time_end;
    std::vector<std::string> images;
    std::string result = "";
    std::ofstream save_result;
    cv::Mat src_img;
    TextNet textnet_process;
    if(textnet_process.init(model_path, input_name, output_name, 0.4f, 11, "AAAABBBBBBB") < 0)
    {
        std::cout << "TextNet init fail!" << std::endl;
        return;
    }
    ListImages(image_dir, images);
    std::cout << "total Test images : " << images.size() << std::endl;
    save_result.open("./text_result.txt");
    for (size_t index = 0; index < images.size(); index++) {
		std::stringstream temp_str;
        temp_str << image_dir << images[index];
		std::cout << temp_str.str() << std::endl;
		src_img = cv::imread(temp_str.str());
        time_start = get_current_time();
        result = textnet_process.run(src_img);
        std::cout << result << std::endl;
        time_end = get_current_time();
        std::cout << "textnet cost time: " <<  (time_end - time_start)/1000.0  << "ms" << std::endl;
        save_result << images[index] << "|" << result << "\n";
    }
    save_result.close();
}

static void image_txt_infer(const std::string &image_dir, const std::string &image_txt_path){
    unsigned long time_start, time_end;
    std::string result = "";
    std::ofstream save_result;
    std::ifstream read_txt;
    std::string line_data;
    cv::Mat src_img;
    TextNet textnet_process;
    if(textnet_process.init(model_path, input_name, output_name, 0.4f, 11, "AAAABBBBBBB") < 0){
        std::cout << "TextNet init fail!" << std::endl;
        return;
    }

    read_txt.open(image_txt_path.data());
    if(!read_txt.is_open()){
        std::cout << image_txt_path << " not exits" << std::endl;
        return;
    }
    
    save_result.open("./text_result.txt");
    while(std::getline(read_txt, line_data)){
        if(line_data.empty()){
            continue;
        }
        size_t index = line_data.find_first_of('|', 0);
        std::string image_name = line_data.substr(0, index);
        std::stringstream image_path;
        image_path << image_dir << image_name;
        std::cout << image_path.str() << std::endl;
        src_img = cv::imread(image_path.str());
        time_start = get_current_time();
        result = textnet_process.run(src_img);
        time_end = get_current_time();
        std::cout << "textnet cost time: " <<  (time_end - time_start) / 1000.0  << "ms" << std::endl;

        save_result << image_name << "|" << result << "\n";
    }
    read_txt.close();
    save_result.close();
}

int main()
{
    std::cout << "TextNet start..." << std::endl;
    const std::string image_dir = "./JPEGImages/";
    const std::string image_txt_path = "./val.txt";
    image_txt_infer(image_dir, image_txt_path);
    // std::string image_dir = "./test_images/";
    // image_dir_infer(image_dir);
    std::cout << "TextNet End of game!!!" << std::endl;
    return 0;
}
