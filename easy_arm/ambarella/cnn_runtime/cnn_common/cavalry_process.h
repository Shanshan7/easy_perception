#ifndef CAVLRYPROCESS_H
#define CAVLRYPROCESS_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "cnn_runtime/cnn_common/common_log.h"
#include "cnn_runtime/cnn_common/cnn_data_structure.h"

/**
 * @brief submodule to operate on cavalry driver
 */

int cavalry_init_context(cavalry_ctx_t *cavalry_ctx, uint8_t verbose);

void cavalry_deinit_context(cavalry_ctx_t *cavalry_ctx);

void cavalry_sync_cache(unsigned long size, unsigned long phys,
                        uint8_t clean, uint8_t invalid);
#endif //CAVLRYPROCESS_H