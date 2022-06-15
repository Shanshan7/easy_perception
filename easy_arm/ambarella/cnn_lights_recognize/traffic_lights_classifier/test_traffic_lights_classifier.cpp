#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include "traffic_lights_classifier.h"
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>



int main()
{
    
//    string img_path="/Users/zhangzikai/Downloads/traffic_light_dataset/JPEGImages/000red/traffic_light_0001.jpg";
    string video_path = "/home/lpj/Desktop/traffic_light_mp4/4.mp4";
    string txt_path="/home/lpj/Desktop/easy_ml_inference/cnn_lights_recognize/4.txt";
    
    TrafficLightsClassifier traffic_lights_classifier;

    cv::VideoCapture cap(video_path);
    cv::Mat frame;
    
    std::ifstream ReadFile;
    ReadFile.open(txt_path,ios::in);
    
    vector<vector<float>> boxes;
    std::string out;
    

    if(ReadFile.fail())
        {
            return 0;
        }
    else{
        while(!ReadFile.fail()){
            getline(ReadFile, out, '\n');
            // std::cout<<out<<std::endl;
    
            std::istringstream ss(out);
        
            vector<float> nums;
            std::string num;

            while(ss >> num) {
                nums.push_back(stoi(num));
            }
//            std::cout<<nums[0]<<std::endl;
            boxes.push_back(nums);
            // break;
        
        }

        boxes.pop_back();
        std::cout<<"boxes size:";
        std::cout<<boxes.size()<<std::endl;
    }
//
    ReadFile.close();

    int frame_num=cap.get(cv::CAP_PROP_FRAME_COUNT);
    std::cout<<"frame_num:";
    std::cout<<frame_num<<std::endl;

    // int epoches=10;
    // for(int epoch=0;epoch<epoches;epoch++){

    int i=0;

    while (cap.isOpened()){
        cap>>frame;
        // std::cout<<"epoch: "+to_string(epoch)<<std::endl;

        if(frame.empty()){
            std::cout<<"frame is empty!"<<std::endl;
            break;
        }

        if(i>0&i<frame_num-1){

            std::cout<<"frame:";
            std::cout<<i<<std::endl;

            vector<TrafficLightsParams> result=traffic_lights_classifier.traffic_lights_result(frame, boxes[i],true,true);
            
            std::string text = to_string(result[0].traffic_lights_type);

            std::cout<<"target_ID:";
            std::cout<<text<<std::endl;

            // cv::putText(frame,text, cv::Point(8, 40), cv::FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);

            // cv::imshow("frame", frame);
            // cv::waitKey(0);

            
        }
        i++;   
    }
    // }
    std::cout<<"finished"<<std::endl;
    // cap.release();
    return 0; 
}
