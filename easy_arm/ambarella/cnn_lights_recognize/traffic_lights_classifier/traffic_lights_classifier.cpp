#include "traffic_lights_classifier.h"


TrafficLightsClassifier::TrafficLightsClassifier()
{
    traffic_lights_results.clear();

    std::ifstream in(json_path, std::ios::binary);
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
        amba_shape=root["amba_shape"].asInt();
        w1=root["w1"].asDouble();
        w2=root["w2"].asDouble();
        amba_path=root["amba_path"].asString();
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

double ** TrafficLightsClassifier::combine_circles(std::vector<cv::Vec3f> circles,cv::Mat image){
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

std::vector<double> TrafficLightsClassifier::estimate_label(double *result[4]){
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

    std::vector<double> label_values {sum_off,sum_red,sum_green,sum_yellow};
//    int maxPosition = max_element(label_values.begin(),label_values.end()) - label_values.begin();
//
//    return maxPosition-1;
    
    std::vector<double> opencv_preds={0,0,0,0};
    int suum=accumulate(label_values.begin(),label_values.end(),0);

    if(suum!=0){
        for(int i=0;i<label_values.size();i++){
            opencv_preds[i]=label_values[i]/suum;
        }
    }
    
    return opencv_preds;
}

std::vector<cv::Vec3f> TrafficLightsClassifier::hough_circles(cv::Mat gray){
    std::vector<cv::Vec3f> circles;
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

TrafficLightsParams TrafficLightsClassifier::traffic_lights_result(cv::Mat image,const std::vector<float> traffic_lights_locations,bool amba,bool opencv){
    cv::Mat res_img,gray,rgb_image_roi,amba_img;
    rgb_image_roi = image(cv::Rect(traffic_lights_locations[0], traffic_lights_locations[1], traffic_lights_locations[2], \
        traffic_lights_locations[3]));

    std::vector<double> opencv_preds={0,0,0,0};
    std::vector<double> combine_preds={0,0,0,0};

    int label_value;
    
    if(!opencv&!amba){
        std::cout<<"No module used!"<<std::endl;
        exit(-1);
    }

    

    if(opencv){

        std::cout<<"opencv start!"<<std::endl;

        resize(rgb_image_roi,res_img,cv::Size(opencv_shape,opencv_shape));
        cvtColor(res_img, gray, cv::COLOR_RGBA2GRAY);
        std::vector<cv::Vec3f> circles;
        circles=hough_circles(gray);

        if(!circles.empty()){
            double **result=combine_circles(circles, res_img);
            opencv_preds=estimate_label(result);
        }

        std::cout<<"opencv end!"<<std::endl;
    }

    
  
    if(amba){

        std::cout<<"amba start!"<<std::endl;

        resize(rgb_image_roi,amba_img,cv::Size(amba_shape,amba_shape));
        Amba_Inference AmbaInference;
        std::vector<float> amba_preds=AmbaInference.amba_pred(amba_img,amba_path);
        
        //if off is None
        amba_preds.insert(amba_preds.begin(),0);
            
        for(int i=0;i<opencv_preds.size();i++){
            combine_preds[i]=w1*opencv_preds[i]+w2*amba_preds[i];
        }
        label_value=max_element(combine_preds.begin(),combine_preds.end()) - combine_preds.begin()-1;

        std::cout<<"amba end!"<<std::endl;
    }
    
    else{
        if(accumulate(opencv_preds.begin(),opencv_preds.end(),0)==0){
            label_value=-1;
        }
        label_value=max_element(opencv_preds.begin(),opencv_preds.end()) - opencv_preds.begin()-1;
    }
        
        
    
    
    TRAFFIC_LIGHTS_TYPE label=TRAFFIC_LIGHTS_TYPE(label_value);
    int target_id =0;
    
    TrafficLightsParams traffic_lights_parma;
    traffic_lights_parma.target_id = target_id;
    traffic_lights_parma.traffic_lights_type = label;
    traffic_lights_parma.traffic_lights_location[0] = traffic_lights_locations[0];
    traffic_lights_parma.traffic_lights_location[1] = traffic_lights_locations[1];
    traffic_lights_parma.traffic_lights_location[2] = traffic_lights_locations[2];
    traffic_lights_parma.traffic_lights_location[3] = traffic_lights_locations[3];

    // std::vector<TrafficLightsParams> res={{target_id,label,traffic_lights_locations}};
    return traffic_lights_parma;
}


