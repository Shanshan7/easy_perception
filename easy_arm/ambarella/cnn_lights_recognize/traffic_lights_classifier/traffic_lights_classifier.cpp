#include "traffic_lights_classifier.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include "json/json.h"
#include <fstream>
#include <numeric>
#include "amba_inference.h"



TrafficLightsClassifier::TrafficLightsClassifier()
{
    traffic_lights_results.clear();

    std::ifstream in(json_path, ios::binary);
    Json::Reader reader;
    Json::Value root;
//
    if(reader.parse(in, root))
    {
        low_green=root["low_green"].asInt();
        up_green=root["up_green"].asInt();
        low_yellow=root["low_yellow"].asInt();
        up_yellow=root["up_yellow"].asInt();
        low_red=root["low_red"].asInt();
        up_red=root["up_red"].asInt();
        low_off=root["low_off"].asInt();
        up_off=root["up_off"].asInt();
        opencv_shape=root["opencv_shape"].asInt();
        onnx_shape=root["onnx_shape"].asInt();
        w1=root["w1"].asDouble();
        w2=root["w2"].asDouble();
        onnx_path=root["onnx_path"].asString();
    }


    else
    {
        std::cout << "Error opening file\n";
        exit(0);
    }

    in.close();
}

TrafficLightsClassifier:: ~TrafficLightsClassifier()
{

}

double * TrafficLightsClassifier::red_green_yellow(const cv::Mat &rgb_image)
{
    double *M=(double *)malloc(4*sizeof(double));
    
    cv::Mat resize_rgb_image, hsv_image;
//    rgb_image_roi = rgb_image(cv::Rect(traffic_lights_locations[0], traffic_lights_locations[1], traffic_lights_locations[2], \
//                                        traffic_lights_locations[3]));
    cv::resize(rgb_image, resize_rgb_image, cv::Size(opencv_shape, opencv_shape));
    cv::cvtColor(resize_rgb_image, hsv_image, cv::COLOR_RGB2HSV);
    std::vector<cv::Mat> hsv_split;
    cv::split(hsv_image, hsv_split);
    cv::Scalar sum_saturation = cv::sum(hsv_split[2]);  // Sum the brightness values
    int area = opencv_shape * opencv_shape;
    float avg_saturation = sum_saturation[0] / area;

    int sat_low = (int)(avg_saturation * 1.3);
    int val_low = 140;

    cv::Mat green_mask, yellow_mask, red_mask, off_mask;
    // Green
    cv::Scalar lower_green = cv::Scalar(low_green, sat_low, val_low);
    cv::Scalar upper_green = cv::Scalar(up_green, 255, 255);
    inRange(hsv_image, lower_green, upper_green, green_mask);
    double sum_green = countNonZero(green_mask);

    // Yellow
    cv::Scalar lower_yellow = cv::Scalar(low_yellow, sat_low, val_low);
    cv::Scalar upper_yellow = cv::Scalar(up_yellow, 255, 255);
    inRange(hsv_image, lower_yellow, upper_yellow, yellow_mask);
    double sum_yellow = countNonZero(yellow_mask);

    // Red
    cv::Scalar lower_red = cv::Scalar(low_red, sat_low, val_low);
    cv::Scalar upper_red = cv::Scalar(up_red, 255, 255);
    inRange(hsv_image, lower_red, upper_red,red_mask);
    double sum_red = countNonZero(red_mask);

    // Off
    cv::Scalar lower_off = cv::Scalar(low_off, sat_low, val_low);
    cv::Scalar upper_off = cv::Scalar(up_off, 255, 46);
    inRange(hsv_image, lower_off, upper_off, off_mask);
    double sum_off = countNonZero(off_mask);

    M[0]=sum_red;
    M[1]=sum_yellow;
    M[2]=sum_green;
    M[3]=sum_off;
    return M;
    
}

double ** TrafficLightsClassifier::combine_circles(vector<cv::Vec3f> circles,cv::Mat image){
    double** result=new double*[circles.size()];
    for(int i=0;i<circles.size();i++){
        result[i]=new double[4];
    }
    
    int y_min,y_max,x_min,x_max;
    for(size_t i=0;i<circles.size();i++){
        y_min=(int(circles[i][1])-int(circles[i][2]))>=0 ? int(circles[i][1])-int(circles[i][2]) : 0;
        y_max=(int(circles[i][1])+int(circles[i][2]))<=image.size[0] ? int(circles[i][1])+int(circles[i][2]) : image.size[0];
        x_min=(int(circles[i][0])-int(circles[i][2]))>=0 ? int(circles[i][0])-int(circles[i][2]) : 0;
        x_max=(int(circles[i][0])+int(circles[i][2]))<=image.size[1] ? int(circles[i][0])+int(circles[i][2]) : image.size[1];
        cv::Rect rect(x_min,y_min,x_max-x_min,y_max-y_min);
        cv::Mat image_roi = image(rect);   //rect既是要截取的区域
        double* res=red_green_yellow(image_roi);
        for(int j=0;j<4;j++){
            result[i][j]=res[j];
        }
    }
    return result;
}

vector<double> TrafficLightsClassifier::estimate_label(double *result[4]){
    double sum_red=0;
    double sum_yellow=0;
    double sum_green=0;
    double sum_off=0;

    for (int i=0;i<sizeof(result[0]) / sizeof(result[0][0]);i++){
        sum_red+=result[i][0];
        sum_yellow+=result[i][1];
        sum_green+=result[i][2];
        sum_off+=result[i][3];
    }

    vector<double> label_values {sum_off,sum_red,sum_green,sum_yellow};
//    int maxPosition = max_element(label_values.begin(),label_values.end()) - label_values.begin();
//
//    return maxPosition-1;
    
    vector<double> opencv_preds={0,0,0,0};
    int suum=accumulate(label_values.begin(),label_values.end(),0);

    if(suum!=0){
        for(int i=0;i<label_values.size();i++){
            opencv_preds[i]=label_values[i]/suum;
        }
    }
    
    return opencv_preds;
}

vector<cv::Vec3f> TrafficLightsClassifier::hough_circles(cv::Mat gray){
    vector<cv::Vec3f> circles;
    HoughCircles(gray, circles,cv::HOUGH_GRADIENT, 1,20,100,10,0,50);
    
    if(circles.empty()){
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
        clahe->setClipLimit(10);
        clahe->setTilesGridSize(cv::Size(3,3));
        cv::Mat imgEquA;
        clahe->apply(gray, imgEquA);
        HoughCircles(gray, circles,cv::HOUGH_GRADIENT, 1,20,100,10,0,50);
    }
    
    return circles;
}

//vector<float> TrafficLightsClassifier::onnx_pred(cv::Mat image,string onnx_path){
//    cv::dnn::Net net = cv::dnn::readNetFromONNX(onnx_path);
//    cv::Mat blob = cv::dnn::blobFromImage(image,1/(std*255),cv::Size(opencv_shape,opencv_shape),cv::Scalar(mean[0],mean[1],mean[2])*255,true,false);  // 由图片加载数据 这里还可以进行缩放、归一化等预处理
//    net.setInput(blob);  // 设置模型输入
//    cv::Mat predict = net.forward(); // 推理出结果
//    return vector<float>(predict);
//}

vector<TrafficLightsParams> TrafficLightsClassifier::traffic_lights_result(cv::Mat image,const vector<float> traffic_lights_locations,bool bin,bool opencv){
    cv::Mat res_img,gray,rgb_image_roi,onnx_img;
    rgb_image_roi = image(cv::Rect(traffic_lights_locations[0], traffic_lights_locations[1], traffic_lights_locations[2], \
        traffic_lights_locations[3]));

    vector<double> opencv_preds={0,0,0,0};
    vector<double> combine_preds={0,0,0,0};

    int label_value;
    
    if(!opencv&!bin){
        std::cout<<"No module used!"<<std::endl;
        exit(-1);
    }

    if(opencv){
        resize(rgb_image_roi,res_img,cv::Size(opencv_shape,opencv_shape));
        cvtColor(res_img, gray, cv::COLOR_RGBA2GRAY);
        vector<cv::Vec3f> circles;
        circles=hough_circles(gray);

        if(!circles.empty()){
            double **result=combine_circles(circles, res_img);
            opencv_preds=estimate_label(result);
        }


    }
  
    if(bin){
        resize(rgb_image_roi,onnx_img,cv::Size(bin_shape,bin_shape));
        AmbaInference AmbaInference;
        std::vector<float> amba_preds=AmbaInference.amba_pred(cv::Mat img,std::string amba_path);
        
        //if off is None
        amba_preds.insert(onnx_preds.begin(),0);
            
        for(int i=0;i<opencv_preds.size();i++){
            combine_preds[i]=w1*opencv_preds[i]+w2*amba_preds[i];
        }
        label_value=max_element(combine_preds.begin(),combine_preds.end()) - combine_preds.begin()-1;
    }
    
    else{
        if(accumulate(opencv_preds.begin(),opencv_preds.end(),0)==0){
            label_value=-1;
        }
        label_value=max_element(opencv_preds.begin(),opencv_preds.end()) - opencv_preds.begin()-1;
    }
        
        
    
    
    TRAFFIC_LIGHTS_TYPE label=TRAFFIC_LIGHTS_TYPE(label_value);
    int target_id =0;
    
    vector<TrafficLightsParams> res={{target_id,label,traffic_lights_locations}};
    return res;
}


