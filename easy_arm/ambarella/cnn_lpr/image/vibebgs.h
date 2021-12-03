/** ViBe is a background subtraction algorithm, described in:
 *
 * Olivier Barnich and Marc Van Droogenbroeck,
 * ViBe: A Universal Background Subtraction Algorithm for Video Sequences,
 * IEEE Transactions on Image Processing, Vol. 20, No. 6, June 2011.
 *
 * M. Van Droogenbroeck and O. Barnich.
 * ViBe: A disruptive method for background subtraction.
 * In T. Bouwmans, F. Porikli, B. Hoferlin, and A. Vacavant, editors,
 * Background Modeling and Foreground Detection for Video Surveillance, chapter 7.
 * Chapman and Hall/CRC, June 2014.
**/
#ifndef VIBEBGS_H
#define VIBEBGS_H

#include <opencv2/core.hpp>

#include "IBGS.h"
#include "IViBeBGS.h"

class ViBeBGS: public IBGS
{

public:
     ViBeBGS();
     ~ViBeBGS();

     void process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel);

private:
     const int DEFAULT_NUM_SAMPLES = 20;
     const int DEFAULT_MATCH_THRESH = 15;
     const int DEFAULT_MATCH_NUM = 2;
     const int DEFAULT_UPDATE_FACTOR = 16;

private:
     int numberOfSamples;
     int matchingThreshold;
     int matchingNumber;
     int updateFactor;
     // vibeModel_Sequential_t* model;

     IViBeBGS *vibe;

     bool firstTime;

private:
     void init();
     void saveConfig();
     void loadConfig();
};

#endif // VIBEBGS_H
