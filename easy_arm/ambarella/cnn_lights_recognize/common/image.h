// #pragma once
#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ialg_open_app.h"
#include "ctx.h"

class Image
{
public:
    Image();
    ~Image();

    void IALG_to_mat(const IALG_IMAGE_INFO_S *pstImage, cv::Mat &output_mat);
};

#endif // _IMAGE_H_