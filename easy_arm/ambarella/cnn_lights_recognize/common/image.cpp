#include "image.h"


Image::Image()
{

}

Image:: ~Image()
{

}

void Image::IALG_to_mat(const IALG_IMAGE_INFO_S *pstImage, cv::Mat &output_mat)
{
    IALG_IMAGE_FORMAT_E enImageFormat = pstImage->enImageFormat;//图片格式
    if (enImageFormat == IALG_IMAGE_FORMAT_E::IALG_PIX_FMT_BGR)
    {
        /* code */
    }
    else if (enImageFormat == IALG_IMAGE_FORMAT_E::IALG_PIX_FMT_YUV420)
    {
        int image_width = pstImage->uiWidth;
        int image_height = pstImage->uiHeight;
        // pstImgInfo->pPlanar[0];//Y分量的虚拟地址
        // pstImgInfo->pPlanar[1];//UV分量的虚拟地址

        int YUV_DATA_H = image_height/2*3;
        cv::Mat YuvI = cv::Mat(YUV_DATA_H, image_width, CV_8UC1);	
        int nCnt0 = image_height*image_width;
        memcpy(YuvI.data, pstImage->pPlanar[0], nCnt0*sizeof(uchar));
        int nCnt1 = (image_height/2)*image_width;
        memcpy(&(YuvI.data[nCnt0]), pstImage->pPlanar[1], nCnt1*sizeof(uchar));		
        output_mat = cv::Mat(image_height, image_width, CV_8UC3);
        cv::cvtColor(YuvI, output_mat, cv::COLOR_YUV2BGR_I420); 
    }
    else if (enImageFormat == IALG_IMAGE_FORMAT_E::IALG_PIX_FMT_NV21)
    {
        /* code */
    }
    else if (enImageFormat == IALG_IMAGE_FORMAT_E::IALG_PIX_FMT_NV12)
    {
        int image_width = pstImage->uiWidth;
        int image_height = pstImage->uiHeight;
        // pstImgInfo->pPlanar[0];//Y分量的虚拟地址
        // pstImgInfo->pPlanar[1];//UV分量的虚拟地址

        int YUV_DATA_H = image_height / 2 * 3;
        cv::Mat YuvI = cv::Mat(YUV_DATA_H, image_width, CV_8UC1);	
        int nCnt0 = image_height*image_width;
        memcpy(YuvI.data, pstImage->pPlanar[0], nCnt0*sizeof(uchar));
        int nCnt1 = (image_height/2)*image_width;
        memcpy(&(YuvI.data[nCnt0]), pstImage->pPlanar[1], nCnt1*sizeof(uchar));		
        output_mat = cv::Mat(image_height, image_width, CV_8UC3);
        cv::cvtColor(YuvI, output_mat, cv::COLOR_YUV2BGR_NV12);
    }
}