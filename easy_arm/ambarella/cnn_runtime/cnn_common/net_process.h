#ifndef NETPROCESS_H
#define NETPROCESS_H

#include "cnn_runtime/cnn_common/common_log.h"
#include "cnn_runtime/cnn_common/cavalry_process.h"

/**
 * @brief submodule to operate on nnctrl lib
 */
#define net_num (1)

int init_net_context(nnctrl_ctx_t *nnctrl_ctx,
                     cavalry_ctx_t *cavalry_ctx,
                     uint8_t verbose, 
                     uint8_t cache_en);

void deinit_net_context(nnctrl_ctx_t *nnctrl_ctx, cavalry_ctx_t *cavalry_ctx);

int init_net(nnctrl_ctx_t *nnctrl_ctx, uint8_t verbose, uint8_t cache_en, uint8_t reuse_mem);

int load_net(nnctrl_ctx_t *nnctrl_ctx);

void set_net_param(nnctrl_ctx_t *nnctrl_ctx, const char* model_path,
                   const char* net_in_name, const char* net_out_name);

void set_net_multi_param(nnctrl_ctx_t *nnctrl_ctx, const char* model_path,
                         const char* net_in_names[], const int in_count,
                         const char* net_out_names[], const int out_count);

int cnn_init(nnctrl_ctx_t *nnctrl_ctx, cavalry_ctx_t *cavalry_ctx);

int cnn_run(nnctrl_ctx_t *nnctrl_ctx, float *output[], int output_count);

#endif //NETPROCESS_H