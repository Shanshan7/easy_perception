#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


cv::Mat get_perspective_mat()
{
    cv::Point2f src_points[] = { 
        cv::Point2f(165, 270),
        cv::Point2f(835, 270),
        cv::Point2f(360, 125),
        cv::Point2f(615, 125) };
    
    cv::Point2f dst_points[] = {
        cv::Point2f(165, 270),
        cv::Point2f(835, 270),
        cv::Point2f(165, 30),
        cv::Point2f(835, 30) };
    
    cv::Mat M = cv::getPerspectiveTransform(src_points, dst_points);
    
    return M;
    
}

//
// src: 输入图像
// top：输出图像
// top2：输出图像2
void to_top(const cv::Mat &src, cv::Mat &top, cv::Mat & top2)
{

// 相机内参
    cv::Mat K       = (cv::Mat_<double>(3,3) <<
                       1681.52901998965,    0,  667.945786496189,
                        0,  1510.91314479132,   375.142239238734,
                        0,  0,  1);
//  注：世界坐标 z超下，所以将外参z反了一下
    cv::Mat Rot = (cv::Mat_<double>(4,4) <<
                 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, -1, 0,
                 0, 0, 0, 1);
// 外参旋转
    cv::Mat extrinsic = (cv::Mat_<double>(4, 4) <<
             -0.003859404269123161, -0.9999420008273281, -0.01005484858798479, -0.11,
             0.01403883800665975, 0.009999753335606716, -0.9998514469463201, 1.51,
             0.999894002395309, -0.003999989333341806, 0.01399943067495647, 0.59,
             0, 0, 0, 1) * Rot;
// 添加平移
    cv::Mat T = (cv::Mat_<double>(3,1) << -15, 0, 15);
// 目标顶视图相机虚拟内存那
    cv::Mat K_top = (cv::Mat_<double>(3,3) <<
                       320,  0,   320,
                       0,    320, 320,
                       0,    0,   1);
// 原始相机在地平面里的高度
    double d        = 1.51;
    double invd     = 1.0 / d;
// 原始相机坐标系中，地平面的法向量
    cv::Mat n       = extrinsic(cv::Rect(0, 0, 3, 3)) * (cv::Mat_<double>(3,1) << 0, 0, 1);
    std::cout << "n = " << std::endl << n << std::endl;
    cv::Mat nT;
    cv::transpose(n, nT);
    cv::Mat H       = extrinsic(cv::Rect(0, 0, 3, 3)).inv() + T * nT * invd;
    cv::Mat Homography     = K_top * H * K.inv();

    cv::warpPerspective(src, top2, Homography, cv::Size(640, 640));

    // 另一种方法：展示了透视变换原理的实现
    // 内参取逆，用于将像素坐标转换到原始相机坐标系
    cv::Mat Kinv = K.inv();
    double KI[3][3], Kt[3][3];
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            KI[i][j] = Kinv.at<double>(i, j);
            Kt[i][j] = K_top.at<double>(i, j);
        }
    }
    // 初始化生成图
    top = cv::Mat(640, 640, CV_8UC3, cv::Scalar(0,0,0));

    for(int i = 0; i < src.rows; i++)
    {
        const cv::Vec3b *p = src.ptr<cv::Vec3b>(i);
        for(int j = 0; j < src.cols; j++)
        {
            // 计算每个原始像素在相机坐标系下的坐标
            double x = KI[0][0] * j + KI[0][1] * i + KI[0][2];
            double y = KI[1][0] * j + KI[1][1] * i + KI[1][2];
            double z = KI[2][0] * j + KI[2][1] * i + KI[2][2];
            // 由于相机坐标系下的点坐标不具备唯一性，但是在已知平面上，具备唯一性
            // 可通过此来唯一化坐标，此时用到了 d 和 NT
            if(y !=0)
            {
                x /= -y;
                z /= -y;
                y /= -y;
                // 注：以下为计算过程，此处只是修改了坐标轴方向，将此坐标转换到目标相机坐标系系
                double xx = x           + 0;
                double yy = -z          + 0;
                double zz = y           + 10;
                // to top： 使用目标相机内参转到像素坐标
                double u = Kt[0][0] * xx + Kt[0][1] * yy + Kt[0][2] * zz;
                double v = Kt[1][0] * xx + Kt[1][1] * yy + Kt[1][2] * zz;
                double s = Kt[2][0] * xx + Kt[2][1] * yy + Kt[2][2] * zz;
                if(s != 0)
                {
                    u /= s;
                    v /= s;
                    if(v >=640 || u >= 640 || v < 0 || u < 0) continue;
                    uchar * q = (top.data + (int(v)*640+(int)u)*3);

                    q[0] = p[j][0];
                    q[1] = p[j][1];
                    q[2] = p[j][2];
                }
            }
        }
    }
}


int main()
{
    int rval = 0;

    cv::Mat src_img = cv::imread("/tmp/1.jpg");

    int height = src_img.rows / 2;
    int width = src_img.cols / 2;

    cv::Mat src_resized;
    cv::resize(src_img, src_resized, cv::Size(width, height), cv::INTER_LINEAR);
    cv::imwrite("/tmp/src_resized.jpg", src_resized);

    cv::Point2f src_points[] = { 
        cv::Point2f(0, 0),
        cv::Point2f(width, 0),
        cv::Point2f(0, height),
        cv::Point2f(width, height) };

    cv::Point2f dst_points[] = {
        cv::Point2f(180,162),
        cv::Point2f(618,0),
        cv::Point2f(552,540),
        cv::Point2f(682,464) };

    cv::Mat M = cv::getPerspectiveTransform(src_points, dst_points);

    // perspective is vertical view
    cv::Mat perspective;
	cv::warpPerspective(src_resized, perspective, M, cv::Size(width, height));
    cv::imwrite("/tmp/perspective_img.jpg", perspective);

    return rval;
}