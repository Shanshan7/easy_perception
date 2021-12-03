#include "cnn_runtime/cnn_common/common_log.h"

int gsc_log_level = 0;

void set_log_level(enum LogLevel level)
{
    gsc_log_level = level;
}