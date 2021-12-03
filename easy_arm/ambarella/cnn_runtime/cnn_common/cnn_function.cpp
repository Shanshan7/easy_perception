#include "cnn_runtime/cnn_common/cnn_function.h"

void softmax(const int count, float *x) {
	float max = 0.0;
	float sum = 0.0;
	for (int i = 0; i< count; i++){
        if (max < x[i]) 
            max = x[i];
    }
	for (int i = 0; i< count; i++) {
		x[i] = exp(x[i] - max);
		sum += x[i];
	}
 
	for (int i = 0; i< count; i++){
        x[i] /= sum;
    }
}

float sigmoid(float p)
{
    return 1.0f / (1.0f + (float) exp(-p));
}