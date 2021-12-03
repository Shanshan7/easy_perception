#ifndef CLUSTERING_RECT_H
#define CLUSTERING_RECT_H

#include "imgproc_c/base/image_data_structure.h"

void clusteringRect(const ResultRect *srcRects, const int srcCount, const float eps, ResultRect *dstRects, int *dstCount);

#endif // CLUSTERING_RECT_H

