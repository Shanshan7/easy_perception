#include "cnn_runtime/cnn_common/cavalry_process.h"

int cavalry_init_context(cavalry_ctx_t *cavalry_ctx, uint8_t verbose)
{
    int rval = 0;
    memset((void *)cavalry_ctx, 0, sizeof(cavalry_ctx_t));

    cavalry_ctx->fd_cavalry = open("/dev/cavalry", O_RDWR, 0);

    if (cavalry_ctx->fd_cavalry < 0) {
        DPRINT_ERROR("open /dev/cavalry failed\n");
        rval = -1;
    }

    rval = cavalry_mem_init(cavalry_ctx->fd_cavalry, verbose);

    if (rval < 0) {
        DPRINT_ERROR("cavalry_mem_init failed\n");
    }

    return 0;
}

void cavalry_deinit_context(cavalry_ctx_t *cavalry_ctx)
{
    if (cavalry_ctx->fd_cavalry >= 0) {
        close(cavalry_ctx->fd_cavalry);
    }
}

void cavalry_sync_cache(unsigned long size, unsigned long phys,
                        uint8_t clean, uint8_t invalid)
{
    if (cavalry_mem_sync_cache(size, phys, clean, invalid) < 0) {
        DPRINT_NOTICE("cavalry_mem_sync_cache failed\n");
    }
}