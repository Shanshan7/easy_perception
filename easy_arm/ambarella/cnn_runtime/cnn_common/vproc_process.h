#ifndef VPROCPROCESS_H
#define VPROCPROCESS_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>

#include "vproc.h"

#include "cnn_runtime/cnn_common/common_log.h"
#include "cnn_runtime/cnn_common/cavalry_process.h"

/**
 * @brief submodule to operate on vproc lib
 *
 */

typedef struct cv_mem_s {
    uint8_t *virt;
    unsigned long phys;
    unsigned long size;
} cv_mem_t;

typedef struct vproc_ctx_s {
    uint8_t cache_en;
    uint8_t is_rgb;

    cv_mem_t lib_mem;
    cv_mem_t yuv2rgb_mem;

    int total_cv_mem_size;
    int total_malloc_size;

    vect_desc_t y_desc;
    vect_desc_t uv_desc;
    vect_desc_t rgb_desc;
    vect_desc_t rgb_roi_desc;

    int max_vproc_batch_num;
} vproc_ctx_t;

int vproc_init_context(vproc_ctx_t *vproc_ctx, cavalry_ctx_t *cavalry_ctx,
                       uint8_t is_rgb, uint8_t cache_en, int max_vproc_batch_num);

void vproc_deinit_context(vproc_ctx_t *vproc_ctx);


#endif /* VPROCPROCESS_H */