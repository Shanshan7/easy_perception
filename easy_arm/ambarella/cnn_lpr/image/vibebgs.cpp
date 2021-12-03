#include "vibebgs.h"
#include <iostream>

ViBeBGS::ViBeBGS():
    numberOfSamples(DEFAULT_NUM_SAMPLES),
    matchingThreshold(DEFAULT_MATCH_THRESH),
    matchingNumber(DEFAULT_MATCH_NUM),
    updateFactor(DEFAULT_UPDATE_FACTOR)
{
    init();
    LOG(WARNING) << "ViBeBGS()";
}

ViBeBGS::~ViBeBGS()
{
    // libvibeModel_Sequential_Free(model);
    if(vibe)
    {
        delete vibe;
        vibe = NULL;
    }
    LOG(WARNING) << "~ViBeBGS()";
}

void ViBeBGS::process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel)
{
    if (img_input.empty())
        return;

    img_output = cv::Mat::zeros(img_input.size(), CV_8UC1);
    // img_bgmodel = cv::Mat::zeros(img_input.size(), CV_8UC1);

    if (firstTime)
    {
        /* Initialization of the ViBe model. */
        // libvibeModel_Sequential_AllocInit_8u_C1R(model, img_input.data, img_input.cols, img_input.rows);

        /* Sets default model values. */
        // libvibeModel_Sequential_SetNumberOfSamples(model, numberOfSamples);
        // libvibeModel_Sequential_SetMatchingThreshold(model, matchingThreshold);
        // libvibeModel_Sequential_SetMatchingNumber(model, matchingNumber);
        // libvibeModel_Sequential_SetUpdateFactor(model, updateFactor);

        vibe->initModel(img_input);

        // saveConfig();
        firstTime = false;
    }
    else
    {
        vibe->detectAndUpdate(img_input, img_output);

        // libvibeModel_Sequential_Segmentation_8u_C1R(model, img_input.data, img_output.data);
        // libvibeModel_Sequential_Update_8u_C1R(model, img_input.data, img_output.data);
    }
}

void ViBeBGS::init()
{
    firstTime = true;
    // model = libvibeModel_Sequential_New();
    // loadConfig();

    vibe = new IViBeBGS();
}

void ViBeBGS::saveConfig()
{
    cv::FileStorage fs;
    fs.open("./ViBeBGS.xml", cv::FileStorage::WRITE);

    cv::write(fs, "numberOfSamples", numberOfSamples);
    cv::write(fs, "matchingThreshold", matchingThreshold);
    cv::write(fs, "matchingNumber", matchingNumber);
    cv::write(fs, "updateFactor", updateFactor);

    fs.release();
}

void ViBeBGS::loadConfig()
{
    cv::FileStorage fs;
    fs.open("./ViBeBGS.xml", cv::FileStorage::READ);

    cv::read(fs["numberOfSamples"], numberOfSamples, DEFAULT_NUM_SAMPLES);
    cv::read(fs["matchingThreshold"], matchingThreshold, DEFAULT_MATCH_THRESH);
    cv::read(fs["matchingNumber"], matchingNumber, DEFAULT_MATCH_NUM);
    cv::read(fs["updateFactor"], updateFactor, DEFAULT_UPDATE_FACTOR);

    fs.release();
}
