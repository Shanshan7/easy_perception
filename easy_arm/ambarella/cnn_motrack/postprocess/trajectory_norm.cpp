#include "trajectory_norm.h"
#include <iostream>

NormTrajectory::NormTrajectory()
{
    kernel_velocity_slide = new float[COMPENSATION_LENGTH];
    for (int i = 1; i < COMPENSATION_LENGTH + 1; i++)
    {
        kernel_velocity_slide[i-1] = i;
    }

    kernel_acceleration = new float[10];
    for (int j = 1; j < 11; j++)
    {
        kernel_acceleration[j-1] = j;
    }

    knl_compens = new float[COMPENSATION_LENGTH];
    for (int k = 1; k < COMPENSATION_LENGTH + 1; k++)
    {
        knl_compens[k - 1] = k * k;
    }

    vlo_weight = 200;
    vlo_power = 1;

    acceleration_weight = 200;
    acceleration_bias = 0.02;

    compensation_weights = 1;
}

NormTrajectory::~NormTrajectory()
{
    delete [] kernel_velocity_slide;
    delete [] kernel_acceleration;
    delete [] knl_compens;
}

void NormTrajectory::cyrve_smoothing(std::vector<float> &cyrve_array, float *kernel, int cyrve_array_length, int kernel_size, float *cyrve_smoothing_array)
{
    // if (cyrve_array.size() == 5)
    // {
    //     float aaa = cyrve_array[0] * kernel[0] + cyrve_array[1] * kernel[1] + cyrve_array[2] * kernel[2] + cyrve_array[3] * kernel[3] + \
    //     cyrve_array[4] * kernel[4];
    //     std::cout << "aaaaaaaaaaaaaaaaaa: " << aaa / 55 << std::endl;
    // }
    int output_length = cyrve_array_length + kernel_size - 1;
    conv1d(cyrve_array, kernel, cyrve_array_length, kernel_size, cyrve_smoothing_array);
    int sum_kernel = std::accumulate(kernel, kernel + kernel_size, 0);
    for (int k = 0; k < output_length; k++)
    {
        cyrve_smoothing_array[k] = cyrve_smoothing_array[k] / (float)sum_kernel;
    }
}

/*----------------------------------------------------------
* position_array: [x1(T0) x1(T1) x1(T2) x1(T3)......
*                  y1(T0) y1(T1) y1(T2) y1(T3)......
*                  x2(T0) x2(T1) x2(T2) x2(T3)......
*                  y2(T0) y2(T1) y2(T2) y2(T3)......
*                     .      .      .      .  ......] n * m
-----------------------------------------------------------*/

void NormTrajectory::trajectory_smoothing(std::vector<std::vector<float>> &position_value, std::vector<std::vector<float>> &position_value_slide, int image_width)
{
    // normalized
    std::vector<std::vector<float>> position_value_norm, position_value_norm_slide;
    position_value_norm = martrix_multiply_num(position_value, 1. / (float)image_width);

    for (int i = 0; i < position_value_norm.size(); i++)
    {
        std::vector<float> position_value_row = position_value_norm[i];
        std::vector<float> position_value_norm_1(position_value_row.begin() + COMPENSATION_LENGTH, position_value_row.end());
        std::vector<float> position_value_row_slide(position_value_row.size() - COMPENSATION_LENGTH);
        float *slide_0 = new float[COMPENSATION_LENGTH];
        for (int j = 0; j < COMPENSATION_LENGTH; j++)
        {
            int knl_compens_part_size = j+1;
            float *knl_compens_part = new float[knl_compens_part_size];
            memset(knl_compens_part, 0, sizeof(knl_compens_part)); //注意一定要清零
            memcpy(knl_compens_part, this->knl_compens, knl_compens_part_size*sizeof(float));
            array_reverse(knl_compens_part, knl_compens_part_size);
            std::vector<float> position_value_part(position_value_norm_1.begin(), position_value_norm_1.begin() + min(j + 1, position_value_norm_1.size()));
            
            // for (int m = 0; m < position_value_part.size(); m++)
            // {
            //     std::cout << position_value_part[m] << " ";
            // }
            // for (int n = 0; n < knl_compens_part_size; n++)
            // {
            //     std::cout << knl_compens_part[n] << " ";
            // }

            int output_length_part = position_value_part.size() + knl_compens_part_size - 1;
            float *position_value_smoothing = new float[output_length_part];
            memset(position_value_smoothing, 0, sizeof(position_value_smoothing)); //注意一定要清零
            
            this->cyrve_smoothing(position_value_part, knl_compens_part, position_value_part.size(), \
                                knl_compens_part_size, position_value_smoothing);
            slide_0[j] = position_value_smoothing[j];
            // std::cout << j << " " << slide_0[j] << std::endl;
            // std::cout << std::endl;

            delete [] position_value_smoothing;
            delete [] knl_compens_part;
        }

        int output_length = position_value_norm_1.size() + COMPENSATION_LENGTH - 1;
        float *position_value_slide_1 = new float[output_length];
        memset(position_value_slide_1, 0, sizeof(position_value_slide_1)); //注意一定要清零

        float *knl_compens_reverse  = new float[COMPENSATION_LENGTH];
        memset(knl_compens_reverse, 0, sizeof(knl_compens_reverse)); //注意一定要清零
        memcpy(knl_compens_reverse, this->knl_compens, COMPENSATION_LENGTH*sizeof(float));
        array_reverse(knl_compens_reverse, COMPENSATION_LENGTH);

        this->cyrve_smoothing(position_value_norm_1, knl_compens_reverse, position_value_norm_1.size(), \
                            COMPENSATION_LENGTH, position_value_slide_1);
        
        memcpy(position_value_slide_1, slide_0, COMPENSATION_LENGTH * sizeof(float));
        position_value_row_slide.assign(&position_value_slide_1[0],&position_value_slide_1[output_length - COMPENSATION_LENGTH + 1]);
        position_value_norm_slide.push_back(position_value_row_slide);
        position_value_slide = martrix_multiply_num(position_value_norm_slide, (float)image_width);

        delete [] slide_0;
        delete [] position_value_slide_1;
        delete [] knl_compens_reverse;
    }
}