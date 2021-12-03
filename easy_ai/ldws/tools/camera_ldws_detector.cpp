#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "ldws_detector/LDWS_Interface.h"

#define PI 3.1415926

void mvDrawLD_Result(cv::Mat &Img, LDWS_InitGuid *pLDWSInit)
{
    int i, j, alarm_result;
    cv::Scalar color;
    LDWS_Output *pTest = NULL;

    LDWS_GetResult(&pTest); //pTest need to be free!!!

    //printf("L=%f, X0=%f \n",pTest->Param[0],pTest->Param[1]);
    char buffer[64];
    sprintf(buffer, "L=%0.1f,x=%0.1f,Vangle=%0.4f,Hangle=%0.4f,C0=%0.5f", pTest->Param[0], pTest->Param[1], pTest->Param[2] * 180 / PI, pTest->Param[3] * 180 / PI, pTest->Param[4] * 180 / PI);
    cv::putText(Img, buffer, cvPoint(10, 50), CV_FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

    int vy = LDWS_GetVanishY();
    int Eu1, Ev1, Cx1, Cy1;
    LDWS_Get_inter_Pamer_W(&Eu1, &Ev1, &Cx1, &Cy1); //得到Donnees->Eu,Ev,Cx,Cy
    alarm_result = pTest->alarm_result;

    for (i = 0; i < 6; i++)
    {
        cv::line(Img,
             cvPoint(pLDWSInit->pBoundPoint[i].x, pLDWSInit->pBoundPoint[i].y),
             cvPoint(pLDWSInit->pBoundPoint[i + 1].x, pLDWSInit->pBoundPoint[i + 1].y),
                cv::Scalar(0, 255, 255), 3);
    }

    if (pTest->Route == 1 && pTest->Route_half == 0)
    {
        for (j = 0; j < 2; j++)
        {
            color = cv::Scalar(0, 255, 0);

            if ((j == 0) && (alarm_result == 1)) //左报警
            {
                color = cv::Scalar(0, 0, 255);
            }
            else if ((j == 1) && (alarm_result == 2))                       //右报警
            {
                color = cv::Scalar(0, 0, 255);  //右报警
            }

            if (pTest->Confidence_detection[0] <= pTest->Confidence - KEEP_FRAME)
            {
                color = cv::Scalar(255, 255, 0);
                break;
            }

            for (i = j * (pTest->NB_INTERVALLES); i < (j + 1) * pTest->NB_INTERVALLES; i++)
            {
                //double temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * (pTest->pCaPoint[i].y - Cy1) * 0.5;
                double temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                cv::line(Img, cvPoint(pTest->pCaPoint[i].x - temp, pTest->pCaPoint[i].y), cvPoint(pTest->pCaPoint[i].x + temp, pTest->pCaPoint[i].y), color, 5);

                if ((i % pTest->NB_INTERVALLES) != 0)
                {
                    //temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * ((pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5 - Cy1) * 0.5;
                    temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                    cv::line(Img, cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 - temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 + temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), color, 5);
                }

                /*printf("L=%f,X0=%f, X=%f, Y=%f, Z=%f \n",pTest->Param[0],pTest->Param[1],pTest->p3DCaPoint[i].X,pTest->p3DCaPoint[i].Y,pTest->p3DCaPoint[i].Z);
                                double Xdis;
                                Xdis=LDWS_GetXofWorld_W(pTest->pCaPoint[i].x, pTest->pCaPoint[i].y);
                                printf("Xdis=%f \n",Xdis);*/
                //if(j==0)
                //	printf("dist=%f \n",pTest->p3DCaPoint[i+pTest->NB_INTERVALLES].X-pTest->p3DCaPoint[i].X);
            }
        }

        color = cv::Scalar(0, 255, 0);



        // 左车道线
        if (pTest->Route == 1 && pTest->Confidence_detection[0] > pTest->Confidence - KEEP_FRAME && pTest->Route_L == 1 && pTest->Confidence_detection[4] > pTest->Confidence - KEEP_FRAME)
        {
            for (i = 2 * (pTest->NB_INTERVALLES); i < (2 + 1) * pTest->NB_INTERVALLES; i++)
            {
                //double temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * (pTest->pCaPoint[i].y - Cy1) * 0.5;
                double temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                cv::line(Img, cvPoint(pTest->pCaPoint[i].x - temp, pTest->pCaPoint[i].y), cvPoint(pTest->pCaPoint[i].x + temp, pTest->pCaPoint[i].y), color, 5);
                if ((i % pTest->NB_INTERVALLES) != 0)
                {
                    //temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * ((pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5 - Cy1) * 0.5;
                    temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                    cv::line(Img, cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 - temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 + temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), color, 5);
                }
            }
        }

        // 右车道线
        if (pTest->Route == 1 && pTest->Confidence_detection[0] > pTest->Confidence - KEEP_FRAME && pTest->Route_R == 1 && pTest->Confidence_detection[5] > pTest->Confidence - KEEP_FRAME)
        {
            for (i = 3 * (pTest->NB_INTERVALLES); i < (3 + 1) * pTest->NB_INTERVALLES; i++)
            {
                //double temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * (pTest->pCaPoint[i].y - Cy1) * 0.5;
                double temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                cv::line(Img, cvPoint(pTest->pCaPoint[i].x - temp, pTest->pCaPoint[i].y), cvPoint(pTest->pCaPoint[i].x + temp, pTest->pCaPoint[i].y), color, 5);
                if ((i % pTest->NB_INTERVALLES) != 0)
                {
                    //temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * ((pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5 - Cy1) * 0.5;
                    temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                    cv::line(Img, cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 - temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 + temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), color, 5);
                }
            }
        }

    }


    if (pTest->Route == 2 && (pTest->Confidence_detection[2] > pTest->Confidence - KEEP_FRAME))
    {

        color = cv::Scalar(0, 255, 0);

        if ((alarm_result == 1)) //左报警
        {
            color = cv::Scalar(0, 0, 255);
        }

        for (i = 0; i < pTest->NB_INTERVALLES; i++)
        {
            //double temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * (pTest->pCaPoint[i].y - Cy1) * 0.5;
            double temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

            cv::line(Img, cvPoint(pTest->pCaPoint[i].x - temp, pTest->pCaPoint[i].y), cvPoint(pTest->pCaPoint[i].x + temp, pTest->pCaPoint[i].y), color, 5);

            if ((i % pTest->NB_INTERVALLES) != 0)
            {
                //temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * ((pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5 - Cy1) * 0.5;
                temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                cv::line(Img, cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 - temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 + temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), color, 5);
            }
        }
    }

    if (pTest->Route == 3 && (pTest->Confidence_detection[3] > pTest->Confidence - KEEP_FRAME))
    {

        color = cv::Scalar(0, 255, 0);

        if ((alarm_result == 2))                       //右报警
        {
            color = cv::Scalar(0, 0, 255);  //右报警
        }

        for (i = 1 * (pTest->NB_INTERVALLES); i < (1 + 1) * pTest->NB_INTERVALLES; i++)
        {
            //double temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * (pTest->pCaPoint[i].y - Cy1) * 0.5;
            double temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

            cv::line(Img, cvPoint(pTest->pCaPoint[i].x - temp, pTest->pCaPoint[i].y), cvPoint(pTest->pCaPoint[i].x + temp, pTest->pCaPoint[i].y), color, 5);

            if ((i % pTest->NB_INTERVALLES) != 0)
            {
                //temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * ((pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5 - Cy1) * 0.5;
                temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                cv::line(Img, cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 - temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 + temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), color, 5);
            }

        }
    }

    if (pTest->Route_half == 1)
    {
        for (j = 0; j < 2; j++)
        {
            color = cv::Scalar(0, 255, 0);

            if ((j == 0) && (alarm_result == 1)) //左报警
            {
                color = cv::Scalar(0, 0, 255);
            }
            else if ((j == 1) && (alarm_result == 2))                       //右报警
            {
                color = cv::Scalar(0, 0, 255);  //右报警
            }

            if (pTest->Confidence_detection[1] <= pTest->Confidence - KEEP_FRAME)
            {
                color = cv::Scalar(255, 255, 0);
                break;
            }

            for (i = j * (pTest->NB_INTERVALLES) + 3; i < (j + 1) * pTest->NB_INTERVALLES; i++)
            {
                //double temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * (pTest->pCaPoint[i].y - Cy1) * 0.5;
                double temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                cv::line(Img, cvPoint(pTest->pCaPoint[i].x - temp, pTest->pCaPoint[i].y), cvPoint(pTest->pCaPoint[i].x + temp, pTest->pCaPoint[i].y), color, 5);

                if ((i % pTest->NB_INTERVALLES) != 0)
                {
                    //temp = (cos(pTest->Param[3]) / pTest->Z0) * 0.45 * ((pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5 - Cy1) * 0.5;
                    temp = LDWS_GetXLengthofImage(0.15, pTest->pCaPoint[i].y) * 0.5;

                    cv::line(Img, cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 - temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), cvPoint((pTest->pCaPoint[i].x + pTest->pCaPoint[i - 1].x) * 0.5 + temp, (pTest->pCaPoint[i].y + pTest->pCaPoint[i - 1].y) * 0.5), color, 5);
                }

                /*printf("L=%f,X0=%f, X=%f, Y=%f, Z=%f \n",pTest->Param[0],pTest->Param[1],pTest->p3DCaPoint[i].X,pTest->p3DCaPoint[i].Y,pTest->p3DCaPoint[i].Z);
                                double Xdis;
                                Xdis=LDWS_GetXofWorld_W(pTest->pCaPoint[i].x, pTest->pCaPoint[i].y);
                                printf("Xdis=%f \n",Xdis);*/
                //if(j==0)
                //	printf("dist=%f \n",pTest->p3DCaPoint[i+pTest->NB_INTERVALLES].X-pTest->p3DCaPoint[i].X);
            }
        }
    }
}

int testLD(const char *videoName, const char *ldwsConfigFile, const char* ldwsCustomCalibration)
{
    int uFramSeq = 0;
    cv::Mat frame;
    cv::Mat grayADASMat;

    LDWS_Output *pTest = NULL;

    int imageWidth = 1280;
    int imageHeight = 720;

    cv::VideoCapture capture(videoName);
    if (!capture.isOpened())
    {
        printf("%s can not open!\n", videoName);
        return -1;
    }
    capture.set(CV_CAP_PROP_POS_FRAMES, uFramSeq);

    /* LDWS初始化 */
    LDWS_AllocModel();
    LDWS_Init(ldwsConfigFile, ldwsCustomCalibration);

    LDWS_InitGuid *pLDWSInit = NULL;
    LDWS_Getinit(&pLDWSInit);  //pLDWSInit need to be free!
    int vanishY = LDWS_GetVanishY();

    grayADASMat = cv::Mat::zeros(imageHeight, imageWidth, CV_8UC1);

    while (1)
    {
        capture >> frame;

        if (frame.empty())
        {
            std::cout << "read video fail!" << std::endl;
            break;
        }

        cv::resize(frame, frame, cv::Size(imageWidth, imageHeight));
        cv::cvtColor(frame, grayADASMat, CV_BGR2GRAY);

        LDWS_Tracker(grayADASMat.data);

        LDWS_GetResult(&pTest); //pTest need to be free!!!
        mvDrawLD_Result(frame, pLDWSInit);
        cv::line(frame, cv::Point(0, vanishY), cv::Point(imageWidth, vanishY), cv::Scalar(0, 0, 255), 2);
        cv::imshow("LD", frame);
        if (cv::waitKey(1) == 27)
        {
            break;
        }
        uFramSeq += 1;
    }

    LDWS_Finalization();
    LDWS_FreeResult(&pTest);
    LDWS_Freeinit(&pLDWSInit);
    return 0;
}

int main(int argc, char **argv)
{
    const std::string videoPath = "/home/lpj/github/data/day/Mapa_20180605145011_stream0_1_20.mp4";
    const std::string ldwsConfigFile = "/home/lpj/github/perception_software/ai_modules/ldws/data/param_camera_MaPa.dat";
    const std::string ldwsCustomCalibration = "/home/lpj/github/perception_software/ai_modules/ldws/data/CarCalibration_MaPa.txt";
    testLD(videoPath.c_str(), ldwsConfigFile.c_str(), ldwsCustomCalibration.c_str());
    return 0;
}
