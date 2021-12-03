#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <string>
#include <sstream>

#include <eazyai.h>
#include <opencv2/core.hpp>
EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_VERBOSE);


void tensor2mat(ea_tensor_t *input_tensor, cv::Mat output_mat, int channel_convert) ;

#endif // _UTILS_H_