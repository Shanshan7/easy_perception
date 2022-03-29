#include <iostream>
#include "common/utils.h"


int main()
{
    // float input[] = {1, 2, 3};
    std::vector<float> input = {0.1, 2.2, 3.4, 5.1};
    float kernel[] = {1, 2, 3, 4, 5};

    int input_length = input.size(); // sizeof(input) / sizeof(float);
    int kernel_size = sizeof(kernel) / sizeof(float);

    int output_length = input_length + kernel_size - 1;
    float output[output_length];
    memset(output, 0, sizeof(output)); //注意一定要清零
    conv1d(input, kernel, input_length, kernel_size, output);

    for (int k = 0; k < output_length; k++)
    {
        std::cout << output[k] << " ";
    }
    std::cout << "\n";

    std::vector<float> vec;
    vec.insert(vec.end(), 5, 0.6);
    vec.push_back(0.8);
    for (int k = 0; k < 6; k++)
    {
        std::cout << vec[k] << " ";
    }
    std::cout << "\n";
    return 0;   
}