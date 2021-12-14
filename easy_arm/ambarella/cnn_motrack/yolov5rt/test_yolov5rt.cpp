#include <yolov5.h>
#include <deepsort.h>
#include <calculate_trajectory.h>
#include <utils.h>
#include <glog/logging.h>


static void showDetection(cv::Mat& img, std::vector<DetectBox>& boxes) {
    cv::Mat temp = img.clone();
    for (auto box : boxes) {
        cv::Point lt(box.x1, box.y1);
        cv::Point br(box.x2, box.y2);
        cv::rectangle(temp, lt, br, cv::Scalar(255, 0, 0), 1);
        //std::string lbl = cv::format("ID:%d_C:%d_CONF:%.2f", (int)box.trackID, (int)box.classID, box.confidence);
		//std::string lbl = cv::format("ID:%d_C:%d", (int)box.trackID, (int)box.classID);
		std::string lbl = cv::format("ID:%d_x:%.1f_y:%.1f",(int)box.trackID,(box.x1+box.x2)/2,(box.y1+box.y2)/2);
        cv::putText(temp, lbl, lt, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0,255,0));
    }
    cv::imshow("img", temp);
    cv::waitKey(1);
}

static void showTrajectory(cv::Mat& img, std::map<int, TrajectoryParams> &track_idx_map) {
    cv::Mat temp = img.clone();
    for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
    {
        if(it->second.draw_flag == 1) {
            cv::Point lt(it->second.pedestrian_x_start.back(), it->second.pedestrian_y_start.back());
            cv::Point br(it->second.pedestrian_x_end.back(), it->second.pedestrian_y_end.back());
            cv::rectangle(temp, lt, br, cv::Scalar(255, 0, 0), 1);
            std::string lbl = cv::format("ID:%d_V:%.2f",(int)it->first,it->second.mean_velocity);
            cv::putText(temp, lbl, lt, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0,255,0));
            for (int j = 0; j < it->second.trajectory_position.size(); j++) {
                cv::Point p(it->second.trajectory_position[j].x, it->second.trajectory_position[j].y);
                cv::circle(temp, p, 2, cv::Scalar(0, 255, 0), -1);
            }
        }
    }
    cv::imshow("img", temp);
    cv::waitKey(1);
}

int main(int argc, char** argv)
{
    int rval = 0;
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, "../log/");
    // google::InstallFailureSignalHandler();
	// google::InstallFailureWriter(&SignalHandle); 

    string yolo_path = "../yolov5/yolov5s.onnx";
    string sort_path = "../deepsort/deepsort.onnx";
    LOG(INFO) << "yolo_path: " << yolo_path;
    LOG(INFO) << "sort_path: " << sort_path;

    if(argc != 2)
	{
        printf("usage: ./test_yolov5rt image_dir\n");
        exit(0);
    }
	std::vector<std::string> images;
	ListImages(argv[1], images);
    LOG(INFO) << "total Test images : " << images.size();

    YOLO yolo_model(yolo_path);
    DeepSort* DS = new DeepSort(sort_path, 128, 256, 0);

    // yolo detect
    std::vector<DetectBox> det_results;
    std::map<int, TrajectoryParams> track_idx_map;
	track_idx_map.clear();
    int frame_id = 0;
	for (int index = 0; index < images.size(); index++) 
	{
		std::stringstream temp_str;
     	temp_str << argv[1] << index+1 << ".jpg";
		std::cout << temp_str.str() << std::endl;
        LOG(INFO) << "Read image name " << temp_str.str();
		cv::Mat frame = cv::imread(temp_str.str());

        det_results.clear();

        unsigned long time_start_yolo, time_end_yolo, time_start_sort, time_end_sort;
        time_start_yolo = get_current_time();
        yolo_model.run(frame, det_results);
        time_end_yolo = get_current_time();
        LOG(INFO) << "[yolov5] yolov5 cost time: " <<  (time_end_yolo - time_start_yolo)/1000.0  << "ms";
        LOG(INFO) << "[yolov5] Detect process Done!!!";
        time_start_sort = get_current_time();
        DS->sort(frame, det_results);
        time_end_sort = get_current_time();
        LOG(INFO) << "[deepsort] deepsort cost time: " <<  (time_end_sort - time_start_sort)/1000.0  << "ms";
        LOG(INFO) << "[deepsort] Deepsort process Done!!!";
        // showDetection(frame, det_results);
        frame_id = index;
        calculate_tracking_trajectory(det_results, track_idx_map, frame_id);
        showTrajectory(frame, track_idx_map);
    }
    LOG(INFO) << "All process Done!";

    google::ShutdownGoogleLogging();
    return rval;
}