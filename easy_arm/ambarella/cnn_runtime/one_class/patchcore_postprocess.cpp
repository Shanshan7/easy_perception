#include "patchcore_postprocess.h"

#define KNEIGHBOURS (9)

cv::Mat train_embedding_process(const std::string &embedding_file,
                                const int output_channel)
{
    std::ifstream embedding(embedding_file, std::ios::binary|std::ios::in);
    embedding.seekg(0,std::ios::end);
    int embedding_length = embedding.tellg() / sizeof(float);
    embedding.seekg(0, std::ios::beg);
    float* embedding_coreset = new float[embedding_length];
    embedding.read(reinterpret_cast<char*>(embedding_coreset), sizeof(float) * embedding_length);
    embedding.close();

    cv::Mat embedding_train(embedding_length / output_channel, output_channel, CV_32FC1);

    memcpy(embedding_train.data, embedding_coreset, embedding_length * sizeof(float));

    return embedding_train;
}

cv::Mat reshape_embedding(const float *output,
                          const int out_channel, \ 
                          const int out_height, \
                          const int out_width)
{
    cv::Mat embedding_test(out_height*out_width, out_channel, CV_32FC1);
    for (int h = 0; h < out_height; h++)
    {
        for (int w = 0; w < out_width; w++)
        {
            for (int c = 0; c < out_channel; c++)
            {
                ((float*)embedding_test.data)[h*out_width*out_channel + w*out_channel + c] = 
                    output[c*out_height*out_height + h*out_width + w];
                // memcpy(embedding_test.data + h*out_width*out_channel + w*out_channel + c, \
                // embedding_array.data + c*out_height*out_height + h*out_width + w, sizeof(float));
            }
        }
    }
    return embedding_test;
}

cv::Mat knn_process(const cv::Mat &embedding_train, const cv::Mat &embedding_test)
{
    cv::Mat result, neighborResponses, distances_mat;
    cv::Ptr<cv::ml::KNearest> knn = cv::ml::KNearest::create();
    knn->setDefaultK(KNEIGHBOURS);
    knn->setAlgorithmType(cv::ml::KNearest::BRUTE_FORCE);
    cv::Mat labels(embedding_train.rows, 1, CV_32FC1, cv::Scalar(0.0));

    knn->train(embedding_train, cv::ml::ROW_SAMPLE, labels);

    knn->findNearest(embedding_test, KNEIGHBOURS, result, neighborResponses, distances_mat);
    
    return distances_mat;
}