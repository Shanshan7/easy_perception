#ifndef _TRAJECROTY_NORM_H_
#define _TRAJECROTY_NORM_H_

#include <numeric>
#include <Eigen/Core>
#include <Eigen/Dense>

#include "../common/utils.h"

#define COMPENSATION_LENGTH 25

class NormTrajectory
{
public:
    NormTrajectory();
    ~NormTrajectory();

public:
    void trajectory_smoothing(std::vector<std::vector<float>> &position_value, std::vector<std::vector<float>> &position_value_slide, int image_width);

private:
    void cyrve_smoothing(std::vector<float> &cyrve_array, float *kernel, int cyrve_array_length, int kernel_size, float *cyrve_smoothing_array);

    float *kernel_velocity_slide;
    float *kernel_acceleration;
    float *knl_compens;
    
    int vlo_weight;
    int vlo_power;

    int acceleration_weight;
    int acceleration_bias;

    int compensation_weights;
};

#endif // _TRAJECROTY_NORM_H_