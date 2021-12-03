#include "cnn_runtime/pose/openpose_postprocess.h"
#include <iostream>
#include <set>

const static int nPoints = 18;

const static std::string keypointsMapping[] = {
    "Nose", "Neck",
    "R-Sho", "R-Elb", "R-Wr",
    "L-Sho", "L-Elb", "L-Wr",
    "R-Hip", "R-Knee", "R-Ank",
    "L-Hip", "L-Knee", "L-Ank",
    "R-Eye", "L-Eye", "R-Ear", "L-Ear"
};

const static std::vector<std::pair<int,int>> mapIdx = {
    {31,32}, {39,40}, {33,34}, {35,36}, {41,42}, {43,44},
    {19,20}, {21,22}, {23,24}, {25,26}, {27,28}, {29,30},
    {47,48}, {49,50}, {53,54}, {51,52}, {55,56}, {37,38},
    {45,46}
};

const static std::vector<std::pair<int,int>> posePairs = {
    {1,2}, {1,5}, {2,3}, {3,4}, {5,6}, {6,7},
    {1,8}, {8,9}, {9,10}, {1,11}, {11,12}, {12,13},
    {1,0}, {0,14}, {14,16}, {0,15}, {15,17}, {2,17},
    {5,16}
};

static void getKeyPoints(const cv::Mat& probMap, const double threshold, std::vector<KeyPoint>& keyPoints){
    cv::Mat smoothProbMap;
    cv::Mat maskedProbMap;
    cv::GaussianBlur(probMap, smoothProbMap, cv::Size(3, 3), 0, 0);

    cv::threshold(smoothProbMap, maskedProbMap, threshold, 255, cv::THRESH_BINARY);

    maskedProbMap.convertTo(maskedProbMap,CV_8U,1);

    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(maskedProbMap,contours,cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);

    keyPoints.clear();

    for(int i = 0; i < contours.size();++i){
        cv::Mat blobMask = cv::Mat::zeros(smoothProbMap.rows,smoothProbMap.cols,smoothProbMap.type());

        cv::fillConvexPoly(blobMask,contours[i],cv::Scalar(1));

        double maxVal=0;
        cv::Point maxLoc(-1, -1);

        cv::minMaxLoc(smoothProbMap.mul(blobMask),0,&maxVal,0,&maxLoc);

        keyPoints.push_back(KeyPoint(maxLoc, probMap.at<float>(maxLoc.y,maxLoc.x)));
    }
}

static void populateInterpPoints(const cv::Point& a,const cv::Point& b,int numPoints,std::vector<cv::Point>& interpCoords){
    float xStep = ((float)(b.x - a.x))/(float)(numPoints-1);
    float yStep = ((float)(b.y - a.y))/(float)(numPoints-1);

    interpCoords.push_back(a);

    for(int i = 1; i< numPoints-1;++i){
        interpCoords.push_back(cv::Point(a.x + xStep*i,a.y + yStep*i));
    }

    interpCoords.push_back(b);
}

static void getValidPairs(const std::vector<cv::Mat>& netOutputParts,
                          const std::vector<std::vector<KeyPoint>>& detectedKeypoints,
                          std::vector<std::vector<ValidPair>>& validPairs,
                          std::set<int>& invalidPairs) {

    int nInterpSamples = 6;
    float pafScoreTh = 0.5f;
    float confTh = 0.8f;

    for(int k = 0; k < mapIdx.size();++k ){

        //A->B constitute a limb
        cv::Mat pafA = netOutputParts[mapIdx[k].first];
        cv::Mat pafB = netOutputParts[mapIdx[k].second];

        //Find the keypoints for the first and second limb
        const std::vector<KeyPoint>& candA = detectedKeypoints[posePairs[k].first];
        const std::vector<KeyPoint>& candB = detectedKeypoints[posePairs[k].second];

        int nA = candA.size();
        int nB = candB.size();

        /*
          # If keypoints for the joint-pair is detected
          # check every joint in candA with every joint in candB
          # Calculate the distance vector between the two joints
          # Find the PAF values at a set of interpolated points between the joints
          # Use the above formula to compute a score to mark the connection valid
         */

        if(nA != 0 && nB != 0){
            std::vector<ValidPair> localValidPairs;

            for(int i = 0; i< nA;++i){
                int maxJ = -1;
                float maxScore = -1;
                bool found = false;

                for(int j = 0; j < nB;++j){
                    std::pair<float,float> distance(candB[j].point.x - candA[i].point.x,candB[j].point.y - candA[i].point.y);

                    float norm = std::sqrt(distance.first*distance.first + distance.second*distance.second);

                    if(!norm){
                        continue;
                    }

                    distance.first /= norm;
                    distance.second /= norm;

                    //Find p(u)
                    std::vector<cv::Point> interpCoords;
                    populateInterpPoints(candA[i].point,candB[j].point,nInterpSamples,interpCoords);
                    //Find L(p(u))
                    std::vector<std::pair<float,float>> pafInterp;
                    for(int l = 0; l < interpCoords.size();++l){
                        pafInterp.push_back(
                            std::pair<float,float>(
                                pafA.at<float>(interpCoords[l].y,interpCoords[l].x),
                                pafB.at<float>(interpCoords[l].y,interpCoords[l].x)
                            ));
                    }

                    std::vector<float> pafScores;
                    float sumOfPafScores = 0;
                    int numOverTh = 0;
                    for(int l = 0; l< pafInterp.size();++l){
                        float score = pafInterp[l].first*distance.first + pafInterp[l].second*distance.second;
                        sumOfPafScores += score;
                        if(score > pafScoreTh){
                            ++numOverTh;
                        }

                        pafScores.push_back(score);
                    }

                    float avgPafScore = sumOfPafScores/((float)pafInterp.size());

                    if(((float)numOverTh)/((float)nInterpSamples) > confTh){
                        if(avgPafScore > maxScore){
                            maxJ = j;
                            maxScore = avgPafScore;
                            found = true;
                        }
                    }

                }/* j */

                if(found){
                    localValidPairs.push_back(ValidPair(candA[i].id,candB[maxJ].id,maxScore));
                }

            }/* i */

            validPairs.push_back(localValidPairs);

        } else {
            invalidPairs.insert(k);
            validPairs.push_back(std::vector<ValidPair>());
        }
    }/* k */
}

static void getPersonwiseKeypoints(const std::vector<std::vector<ValidPair>>& validPairs,
                                   const std::set<int>& invalidPairs,
                                   std::vector<std::vector<int>>& personwiseKeypoints) {
    for(int k = 0; k < mapIdx.size();++k){
        if(invalidPairs.find(k) != invalidPairs.end()){
            continue;
        }

        const std::vector<ValidPair>& localValidPairs(validPairs[k]);

        int indexA(posePairs[k].first);
        int indexB(posePairs[k].second);

        for(int i = 0; i< localValidPairs.size();++i){
            bool found = false;
            int personIdx = -1;

            for(int j = 0; !found && j < personwiseKeypoints.size();++j){
                if(indexA < personwiseKeypoints[j].size() &&
                   personwiseKeypoints[j][indexA] == localValidPairs[i].aId){
                    personIdx = j;
                    found = true;
                }
            }/* j */

            if(found){
                personwiseKeypoints[personIdx].at(indexB) = localValidPairs[i].bId;
            } else if(k < 17){
                std::vector<int> lpkp(std::vector<int>(18,-1));

                lpkp.at(indexA) = localValidPairs[i].aId;
                lpkp.at(indexB) = localValidPairs[i].bId;

                personwiseKeypoints.push_back(lpkp);
            }

        }/* i */
    }/* k */
}

void getOpenposeResult(const std::vector<cv::Mat>& netOutputParts, std::vector<std::vector<KeyPoint>> &result){
    int keyPointId = 0;
    std::vector<std::vector<KeyPoint>> detectedKeypoints;
    std::vector<std::vector<ValidPair>> validPairs;
    std::set<int> invalidPairs;
    std::vector<KeyPoint> keyPointsList;
    std::vector<std::vector<int>> personwiseKeypoints;
    keyPointsList.clear();
    personwiseKeypoints.clear();
    result.clear();

    for(int i = 0; i < nPoints;++i){
        std::vector<KeyPoint> keyPoints;
        keyPoints.clear();

        getKeyPoints(netOutputParts[i], 0.25, keyPoints);

        for(int i = 0; i< keyPoints.size();++i,++keyPointId){
            keyPoints[i].id = keyPointId;
        }

        detectedKeypoints.push_back(keyPoints);
        keyPointsList.insert(keyPointsList.end(),keyPoints.begin(),keyPoints.end());
        // std::cout << "keyPoints:" << keyPoints.size() << std::endl;
    }

    getValidPairs(netOutputParts, detectedKeypoints, validPairs, invalidPairs);
    getPersonwiseKeypoints(validPairs, invalidPairs, personwiseKeypoints);

    for(int n  = 0; n < personwiseKeypoints.size();++n){
        std::vector<KeyPoint> objectPoints;
        for(int k = 0; k < personwiseKeypoints[0].size(); ++k){
            int index = personwiseKeypoints[n][k];
            if(index == -1){
                objectPoints.push_back(KeyPoint(cv::Point(-1, -1), -1));
            }
            else {
                objectPoints.push_back(keyPointsList[index]);
            }
        }
        result.push_back(objectPoints);
    }
    std::cout << "result:" << result.size() << std::endl;
}
