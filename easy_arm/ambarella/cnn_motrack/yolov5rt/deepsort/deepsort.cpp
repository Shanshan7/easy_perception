#define _DLL_EXPORTS

#include "deepsort.h"
#include <glog/logging.h>

//DeepSort::DeepSort(std::string modelPath, int batchSize, int featureDim, int gpuID, ILogger* gLogger) {
//    this->gpuID = gpuID;
//    this->enginePath = modelPath;
//    this->batchSize = batchSize;
//    this->featureDim = featureDim;
//    this->imgShape = cv::Size(64, 128);
//    this->maxBudget = 100;
//    this->maxCosineDist = 0.2;
//    this->gLogger = gLogger;
//    init();
//}

DeepSort::DeepSort(std::string modelPath, int batchSize, int featureDim, int gpuID) {
    this->gpuID = gpuID;
    this->enginePath = modelPath;
    this->batchSize = batchSize;
    this->featureDim = featureDim;
    this->imgShape = cv::Size(64, 128);
    this->maxBudget = 100;
    this->maxCosineDist = 0.2;
    init();
}


void DeepSort::init() {
    objTracker = new tracker(maxCosineDist, maxBudget);
    //featureExtractor = new FeatureTensor(batchSize, imgShape, featureDim, gpuID, gLogger);
    featureExtractor = new FeatureTensor(batchSize, imgShape, featureDim, gpuID);

#ifdef FEATURE_MATCH_EN
    featureExtractor->loadOnnx(enginePath);
#endif

}

DeepSort::~DeepSort() {
    delete objTracker;
}

void DeepSort::sort(cv::Mat& frame, vector<DetectBox>& dets) {
    // preprocess Mat -> DETECTION
    DETECTIONS detections;
    vector<CLSCONF> clsConf;

    for (DetectBox i : dets) {
        DETECTBOX box(i.x1, i.y1, i.x2 - i.x1, i.y2 - i.y1);
        DETECTION_ROW d;
        d.tlwh = box;
        d.confidence = i.confidence;
        detections.push_back(d);
        clsConf.push_back(CLSCONF((int)i.classID, i.confidence));
    }
    result.clear();
    results.clear();
    if (detections.size() > 0) {
        DETECTIONSV2 detectionsv2 = make_pair(clsConf, detections);
        sort(frame, detectionsv2);
    }
    // postprocess DETECTION -> Mat
    dets.clear();
    for (auto r : result) {
        DETECTBOX i = r.second;
        DetectBox b(i(0), i(1), i(2) + i(0), i(3) + i(1), 1.);
        b.trackID = (float)r.first;
        dets.push_back(b);
    }
    for (int i = 0; i < results.size(); ++i) {
        CLSCONF c = results[i].first;
        dets[i].classID = c.cls;
        dets[i].confidence = c.conf;
    }
}


void DeepSort::sort(cv::Mat& frame, DETECTIONS& detections) {
#ifdef FEATURE_MATCH_EN
    bool flag = featureExtractor->getRectsFeature(frame, detections);
#else
    bool flag = featureExtractor->getRectsFeature(detections);
#endif
    if (flag) {
        objTracker->predict();
        objTracker->update(detections);
        //result.clear();
        for (Track& track : objTracker->tracks) {
            if (!track.is_confirmed() || track.time_since_update > 1)
                continue;
            result.push_back(make_pair(track.track_id, track.to_tlwh()));
        }
    }
}

void DeepSort::sort(cv::Mat& frame, DETECTIONSV2& detectionsv2) {
    std::vector<CLSCONF>& clsConf = detectionsv2.first;
    DETECTIONS& detections = detectionsv2.second;
#ifdef FEATURE_MATCH_EN
    bool flag = featureExtractor->getRectsFeature(frame, detections);
#else
    bool flag = featureExtractor->getRectsFeature(detections);
#endif
    LOG(INFO) << "[deepsort] Extract REID feature!";
    if (flag) {
        LOG(INFO) << "[deepsort] Start tracking!";
        objTracker->predict();
        LOG(INFO) << "[deepsort] Predict track object done!";
        objTracker->update(detectionsv2);
        LOG(INFO) << "[deepsort] Update object track id done!";
        result.clear();
        results.clear();
        for (Track& track : objTracker->tracks) {
            if (!track.is_confirmed() || track.time_since_update > 1)
                continue;
            result.push_back(make_pair(track.track_id, track.to_tlwh()));
            results.push_back(make_pair(CLSCONF(track.cls, track.conf), track.to_tlwh()));
        }
        LOG(INFO) << "[deepsort] Track object has been record in results done!";
    }
}

void DeepSort::sort(vector<DetectBox>& dets) {
    DETECTIONS detections;
    for (DetectBox i : dets) {
        DETECTBOX box(i.x1, i.y1, i.x2 - i.x1, i.y2 - i.y1);
        DETECTION_ROW d;
        d.tlwh = box;
        d.confidence = i.confidence;
        detections.push_back(d);
    }
    if (detections.size() > 0)
        sort(detections);
    dets.clear();
    for (auto r : result) {
        DETECTBOX i = r.second;
        DetectBox b(i(0), i(1), i(2), i(3), 1.);
        b.trackID = r.first;
        dets.push_back(b);
    }
}

void DeepSort::sort(DETECTIONS& detections) {
    bool flag = featureExtractor->getRectsFeature(detections);
    if (flag) {
        objTracker->predict();
        objTracker->update(detections);
        result.clear();
        for (Track& track : objTracker->tracks) {
            if (!track.is_confirmed() || track.time_since_update > 1)
                continue;
            result.push_back(make_pair(track.track_id, track.to_tlwh()));
        }
    }
}
