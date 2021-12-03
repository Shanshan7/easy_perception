#include "cnn_runtime/cnn_common/vproc_process.h"

int vproc_init_context(vproc_ctx_t *vproc_ctx, cavalry_ctx_t *cavalry_ctx,
                       uint8_t is_rgb, uint8_t cache_en, int max_vproc_batch_num)
{
    int rval = 0;
    struct vproc_version ver;
    uint32_t size = 0;
    memset(vproc_ctx, 0, sizeof(vproc_ctx_t));
    vproc_ctx->is_rgb = is_rgb;
    vproc_ctx->cache_en = cache_en;
    vproc_ctx->max_vproc_batch_num = max_vproc_batch_num;

    rval = vproc_get_version(&ver);

    if (rval < 0) {
        DPRINT_ERROR("vproc_get_version failed\n");
        rval = -1;
    }

    DPRINT_NOTICE("%s: %u.%u.%u, mod-time: 0x%x\n",
                    ver.description, ver.major, ver.minor, ver.patch, ver.mod_time);
    rval = vproc_init("/usr/local/vproc/vproc.bin", &size);

    if (rval < 0) {
        DPRINT_ERROR("vproc_init failed, can not init /usr/local/vproc/vproc.bin\n");
    }

    memset(&vproc_ctx->lib_mem, 0, sizeof(vproc_ctx->lib_mem));
    vproc_ctx->lib_mem.size = size;
    rval = cavalry_mem_alloc(&(vproc_ctx->lib_mem.size), &(vproc_ctx->lib_mem.phys), (void **) & (vproc_ctx->lib_mem.virt), vproc_ctx->cache_en);

    std::cout << "vproc_init_context_cavalry_mem_alloc: " << rval << std::endl;

    if (rval < 0) {
        DPRINT_ERROR("alloc_cv_mem failed\n");
    }

    vproc_ctx->total_cv_mem_size += vproc_ctx->lib_mem.size;
    vproc_load(cavalry_ctx->fd_cavalry, vproc_ctx->lib_mem.virt, vproc_ctx->lib_mem.phys, size);

    if (rval < 0) {
    }

    DPRINT_NOTICE("vproc use cavalry mem total %d bytes\n", vproc_ctx->total_cv_mem_size);
    DPRINT_NOTICE("vproc use malloc total %d bytes\n", vproc_ctx->total_malloc_size);

    return rval;
}

void vproc_deinit_context(vproc_ctx_t *vproc_ctx)
{

}