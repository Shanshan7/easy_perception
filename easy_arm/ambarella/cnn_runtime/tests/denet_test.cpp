#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "utility/utils.h"
#include "cnn_runtime/det2d/denet.h"

#define CLASS_NUMBER (2)

const static std::string model_path = "./denet.bin";
const static std::vector<std::string> input_name = {"data"};
const static std::vector<std::string> output_name = {"det_output0", "det_output1", "det_output2"};
const char* class_name[CLASS_NUMBER] = {"green_strawberry", "strawberry"};

static bool is_file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

static void image_dir_infer(const std::string &image_dir){
    unsigned long time_start, time_end;
    std::vector<std::string> images;
    std::vector<std::vector<float>> boxes;
    cv::Mat src_image;
    DeNet denet_process;
    if(denet_process.init(model_path, input_name, output_name, CLASS_NUMBER) < 0)
    {
        std::cout << "DeNet init fail!" << std::endl;
        return;
    }
    ListImages(image_dir, images);
    std::cout << "total Test images : " << images.size() << std::endl;
    for (size_t index = 0; index < images.size(); index++) {
		std::stringstream temp_str;
        temp_str << image_dir << images[index];
		std::cout << temp_str.str() << std::endl;
        if(!is_file_exists(temp_str.str())){
            std::cout << temp_str.str() << " not exists" << std::endl;
            continue;
        }
        src_image = cv::imread(temp_str.str());
        time_start = get_current_time();
        boxes = denet_process.run(src_image);
        time_end = get_current_time();
        std::cout << "det2d cost time: " <<  (time_end - time_start)/1000.0  << "ms" << std::endl;

        for (size_t i = 0; i < boxes.size(); ++i)
	    {
            float xmin = boxes[i][0];
            float ymin = boxes[i][1];
            float xmax = xmin + boxes[i][2];
            float ymax = ymin + boxes[i][3];
            int type = boxes[i][4];
            float confidence = boxes[i][5];
            std::cout << class_name[type] << " " << confidence << " " << xmin 
                                << " " << ymin << " " << xmax << " " << ymax << "|";
            cv::rectangle(src_image, cv::Point(xmin, ymin), cv::Point(xmax, ymax), cv::Scalar(0, 255, 255), 2, 4);
	    }
        std::cout << std::endl;
        cv::imwrite(images[index], src_image);
    }
}

static void image_txt_infer(const std::string &image_dir, const std::string &image_txt_path){
    unsigned long time_start, time_end;
    std::vector<std::vector<float>> boxes;
    std::ofstream save_result;
    std::ifstream read_txt;
    std::string line_data;
    cv::Mat src_image;
    DeNet denet_process;
    if(denet_process.init(model_path, input_name, output_name, CLASS_NUMBER) < 0)
    {
        std::cout << "DeNet init fail!" << std::endl;
        return;
    }

    read_txt.open(image_txt_path.data());
    if(!read_txt.is_open()){
        std::cout << image_txt_path << " not exits" << std::endl;
        return;
    }
    
    save_result.open("./det2d_result.txt");
    while(std::getline(read_txt, line_data)){
        boxes.clear();
        if(line_data.empty()){
            continue;
        }
        size_t index = line_data.find_first_of(' ', 0);
        std::string image_name = line_data.substr(0, index);
        std::stringstream image_path;
        image_path << image_dir << image_name;
        std::cout << image_path.str() << std::endl;
        if(!is_file_exists(image_path.str())){
            std::cout << image_path.str() << " not exists" << std::endl;
            continue;
        }
        src_image = cv::imread(image_path.str());
        time_start = get_current_time();
        boxes = denet_process.run(src_image);
        time_end = get_current_time();
        std::cout << "det2d cost time: " <<  (time_end - time_start)/1000.0  << "ms" << std::endl;

        save_result << image_name << "|";
        for (size_t i = 0; i < boxes.size(); ++i)
	    {
            float xmin = boxes[i][0];
            float ymin = boxes[i][1];
            float xmax = xmin + boxes[i][2];
            float ymax = ymin + boxes[i][3];
            int type = boxes[i][4];
            float confidence = boxes[i][5];
            save_result << class_name[type] << " " << confidence << " " << xmin 
                                << " " << ymin << " " << xmax << " " << ymax << "|";
            // cv::rectangle(src_image, cv::Point(xmin, ymin), cv::Point(xmax, ymax), cv::Scalar(0, 255, 255), 2, 4);
	    }
        save_result << "\n";
        // cv::imwrite(image_name, src_image);
    }
    read_txt.close();
    save_result.close();
}

static void usage(void)
{
	printf("denet_test usage:\n");
	printf("\nNote:\n"
			"The denet.bin should be given\n");
	printf("\nExample:\n"
		"./denet_test 0 image_dir\n");

	return;
}

int main(int argc, char **argv)
{
    std::cout << "start..." << std::endl;
    // if (argc < 3) {
	// 	usage();
	// 	return -1;
    // }
    const std::string image_dir = "./images/";
    const std::string image_txt_path = "./val.txt";
    image_txt_infer(image_dir, image_txt_path);
    std::cout << "End of game!!!" << std::endl;
    return 0;
}
