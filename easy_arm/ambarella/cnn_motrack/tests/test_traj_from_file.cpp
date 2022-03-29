#include <calculate_trajectory.h>
#include <data_struct.h>

#include <iostream>

static std::vector<DetectResultInfo> allDetections;

static void split(const std::string& s, std::vector<std::string>& token, char delim=' ') {
    token.clear();
    auto string_find_first_not = [s, delim](size_t pos = 0) -> size_t {
        for (size_t i = pos; i < s.size(); ++i)
            if (s[i] != delim) return i;
        return std::string::npos;
    };
    size_t lastPos = string_find_first_not(0);
    size_t pos = s.find(delim, lastPos);
    while (lastPos != std::string::npos) {
        token.emplace_back(s.substr(lastPos, pos-lastPos));
        lastPos = string_find_first_not(pos);
        pos = s.find(delim, lastPos);
    }
}

static void loadDetections(std::string txtPath) {
    //fstream f(filePath, ios::in);
    // this->txtPath = txtPath;
    std::ifstream inFile;
    inFile.open(txtPath, std::ios::binary);
    std::string temp;
    std::vector<std::string> token;
    while (getline(inFile, temp)) {
        // std::cout << temp << std::endl;
        split(temp, token, ' ');
        int frame = atoi(token[0].c_str());
        int id     = atoi(token[1].c_str());
        int x1     = atoi(token[2].c_str());
        int y1     = atoi(token[3].c_str());
        int x2     = atoi(token[4].c_str());
        int y2     = atoi(token[5].c_str());
        float conf = atof(token[6].c_str());     
        while (allDetections.size() <= frame) {
            std::vector<DetectResult> t;
            DetectResultInfo det_result_info;
            det_result_info.detect_result_vector = t;
            det_result_info.current_frame = frame;
            allDetections.push_back(det_result_info);
        }
        DetectResult dd;
        dd.track_id = id;
        dd.class_id = 1;
        dd.confidence = conf;
        dd.pedestrian_location[0] = x1;
        dd.pedestrian_location[1] = y1;
        dd.pedestrian_location[2] = x2;
        dd.pedestrian_location[3] = y2;
        // DetectBox dd(x-w/2, y-h/2, x+w/2, y+h/2, con, c);
        allDetections[frame].detect_result_vector.push_back(dd);
    }
    allDetections.pop_back();
}

static void showTrajectory(cv::Mat& img, std::map<int, TrajectoryParams> &track_idx_map) {
    // cv::Mat temp = img.clone();
    for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
    {
        if(it->second.draw_flag == 1) {
            cv::Point lt(it->second.pedestrian_location[0], it->second.pedestrian_location[1]);
            cv::Point br(it->second.pedestrian_location[2], it->second.pedestrian_location[3]);
            cv::rectangle(img, lt, br, cv::Scalar(255, 0, 0), 1);
            std::string lbl = cv::format("ID:%d D:%d V:%.2f P:%.2f",(int)it->first, it->second.object_direction, it->second.mean_velocity, it->second.relative_distance);
            cv::putText(img, lbl, lt, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0,255,0));
            for (int j = 0; j < it->second.trajectory_position.size(); j++) {
                cv::Point p(it->second.trajectory_position[j].x, it->second.trajectory_position[j].y);
                cv::circle(img, p, 2, cv::Scalar(0, 255, 0), -1);
            }
        }
    }
}

int main(int argc, char **argv) 
{
    int rval = 0;

    if (argc < 4) {
        std::cout << "./test_traj_from_file [input image path] [input txt path] [save_result_path]" << std::endl;
        return -1;
    }

    CalculateTraj calculate_traj;
#ifdef IS_SAVE_DATA
    calculate_traj.init_save_dir();
#endif

    loadDetections(argv[2]);
    for (DetectResultInfo det : allDetections) 
    {
        if (det.detect_result_vector.size() > 0)
        {
            int current_frame = det.current_frame;
            std::stringstream image_path;
            image_path.str("");
            image_path << argv[1] << current_frame << ".jpg";
            std::cout << image_path.str() << std::endl;

#ifdef IS_SAVE_DATA
            calculate_traj.save_det_result(det);
#endif

            cv::Mat image = cv::imread(image_path.str());
            det.yuv_data.data_width = image.cols;
            det.yuv_data.data_height = image.rows;
            calculate_traj.calculate_trajectory(det);
            showTrajectory(image, calculate_traj.track_idx_map);
            
            std::stringstream save_result_path;
            save_result_path.str("");
            save_result_path << argv[3] << current_frame << ".jpg";
            cv::imwrite(save_result_path.str(), image);
        }
    }

    return rval;
}