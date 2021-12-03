#include "cnn_runtime/cnn_common/net_process.h"

const static int g_canvas_id = 1;
static int nnctrl_first_init = 1;

static int init_net_memory(struct net_cfg *cfg, struct net_input_cfg *input_cfg, struct net_output_cfg *ouput_cfg, struct net_mem *mem,
                           uint8_t verbose, uint8_t reuse_mem, uint32_t batch_num, uint8_t cache_en, char *model_file_path){
    int rval = 0;
    int net_id = 0;

    cfg->net_file = model_file_path;
    cfg->verbose = verbose;
    cfg->reuse_mem = reuse_mem;
    cfg->net_loop_cnt = batch_num;

    net_id = nnctrl_init_net(cfg, input_cfg, ouput_cfg);

    if (net_id < 0) {
        DPRINT_ERROR("nnctrl_init_net failed for %s\n", model_file_path);
    }

    if (cfg->net_mem_total == 0) {
        DPRINT_ERROR("nnctrl_init_net get total size is zero for %s\n", model_file_path);
        net_id = -1;
    }

    mem->mem_size = cfg->net_mem_total;
    unsigned long size = mem->mem_size;
    unsigned long phy_addr;

    rval = cavalry_mem_alloc(&size, &phy_addr, (void **) & (mem->virt_addr),
                                cache_en);

    mem->mem_size = size;
    mem->phy_addr = phy_addr;

    if (rval < 0) {
        DPRINT_ERROR("cavalry_mem_alloc failed\n");
        net_id = -1;
    }

    if (mem->virt_addr == NULL) {
        DPRINT_ERROR("cavalry_mem_alloc is NULL\n");
        net_id = -1;
    }

    return net_id;
}

int init_net_context(nnctrl_ctx_t *nnctrl_ctx,
                     cavalry_ctx_t *cavalry_ctx,
                     uint8_t verbose, 
                     uint8_t cache_en){
    int rval = 0;
    struct nnctrl_version ver;

    set_log_level((enum LogLevel)(nnctrl_ctx->log_level));

    rval = cavalry_init_context(cavalry_ctx, nnctrl_ctx->verbose);

    if (rval < 0) {
        printf("cavalry init error, return %d\n", rval);
    }

    nnctrl_ctx->cache_en = cache_en;
    rval = nnctrl_get_version(&ver);

    if (rval < 0) {
        printf("nnctrl_get_version failed");
    }

    DPRINT_NOTICE("%s: %u.%u.%u, mod-time: 0x%x\n",
                    ver.description, ver.major, ver.minor, ver.patch, ver.mod_time);
    if(nnctrl_first_init == 1){
        rval = nnctrl_init(cavalry_ctx->fd_cavalry, verbose);
        if (rval < 0) {
            printf("nnctrl_init failed\n");
        }
        else{
            nnctrl_first_init = 0;
        }
    }
    else{
        printf("nnctrl_init not first\n");
    }
    return rval;
}

void deinit_net_context(nnctrl_ctx_t *nnctrl_ctx, cavalry_ctx_t *cavalry_ctx){
    unsigned long size;
    unsigned long phy_addr;
    if (nnctrl_ctx->net.net_m.virt_addr && nnctrl_ctx->net.net_m.mem_size) {
        size = nnctrl_ctx->net.net_m.mem_size;
        phy_addr = nnctrl_ctx->net.net_m.phy_addr;
        if (cavalry_mem_free(size, phy_addr, nnctrl_ctx->net.net_m.virt_addr) < 0) {
                DPRINT_NOTICE("cavalry_mem_free failed\n");
            }
    }
    cavalry_deinit_context(cavalry_ctx);
}

int init_net(nnctrl_ctx_t *nnctrl_ctx, uint8_t verbose, uint8_t cache_en, uint8_t reuse_mem){
    int rval;

    //net init
    nnctrl_ctx->net.net_id = -1;

    rval = init_net_memory(&nnctrl_ctx->net.cfg, &nnctrl_ctx->net.net_in, &nnctrl_ctx->net.net_out,
                           &nnctrl_ctx->net.net_m, verbose, reuse_mem, 0/*posenet_batch_num*/, cache_en, nnctrl_ctx->net.net_file);

    nnctrl_ctx->net.net_id = rval;

    return rval;
}

int load_net(nnctrl_ctx_t *nnctrl_ctx){
    int rval = 0;

    // load net start
    rval = nnctrl_load_net(nnctrl_ctx->net.net_id, &nnctrl_ctx->net.net_m,
                           &nnctrl_ctx->net.net_in, &nnctrl_ctx->net.net_out);

    if (nnctrl_ctx->cache_en) {
        cavalry_sync_cache(nnctrl_ctx->net.net_m.mem_size, nnctrl_ctx->net.net_m.phy_addr, 1, 0);
    }
    // load net end

    if (rval < 0)
    {
        DPRINT_ERROR("nnctrl_load_all_net error, return %d\n", rval);
    }

    return rval;
}

void set_net_param(nnctrl_ctx_t *nnctrl_ctx, const char* model_path,
                   const char* net_in_name, const char* net_out_name)
{
    memset(nnctrl_ctx, 0, sizeof(nnctrl_ctx_t));

    nnctrl_ctx->verbose = 0;
    nnctrl_ctx->reuse_mem = 1;
    nnctrl_ctx->cache_en = 1;
    nnctrl_ctx->buffer_id = g_canvas_id;
    nnctrl_ctx->log_level = 0;

    strcpy(nnctrl_ctx->net.net_file, model_path); 

    nnctrl_ctx->net.net_in.in_num = 1;
    nnctrl_ctx->net.net_in.in_desc[0].name = net_in_name;
	nnctrl_ctx->net.net_in.in_desc[0].no_mem = 0;

	nnctrl_ctx->net.net_out.out_num = 1;
    nnctrl_ctx->net.net_out.out_desc[0].name = net_out_name; 
	nnctrl_ctx->net.net_out.out_desc[0].no_mem = 0; // let nnctrl lib allocate memory for output
}

void set_net_multi_param(nnctrl_ctx_t *nnctrl_ctx, const char* model_path,
                         const char* net_in_names[], const int in_count,
                         const char* net_out_names[], const int out_count)
{
    memset(nnctrl_ctx, 0, sizeof(nnctrl_ctx_t));

    nnctrl_ctx->verbose = 0;
    nnctrl_ctx->reuse_mem = 1;
    nnctrl_ctx->cache_en = 1;
    nnctrl_ctx->buffer_id = g_canvas_id;
    nnctrl_ctx->log_level = 0;

    strcpy(nnctrl_ctx->net.net_file, model_path); 

    nnctrl_ctx->net.net_in.in_num = in_count;
    for (int i = 0; i < in_count; i++)
    {
        nnctrl_ctx->net.net_in.in_desc[i].name = net_in_names[i];
	    nnctrl_ctx->net.net_in.in_desc[i].no_mem = 0;
    }
    nnctrl_ctx->net.net_out.out_num = out_count;
    for (int i = 0; i < out_count; i++)
    {
        nnctrl_ctx->net.net_out.out_desc[i].name = net_out_names[i];
	    nnctrl_ctx->net.net_out.out_desc[i].no_mem = 0; // let nnctrl lib allocate memory for output
    }
}

int cnn_init(nnctrl_ctx_t *nnctrl_ctx, cavalry_ctx_t *cavalry_ctx)
{
    int rval = 0;
    rval = init_net_context(nnctrl_ctx, cavalry_ctx, 
                            nnctrl_ctx->verbose, nnctrl_ctx->cache_en);

    rval = init_net(nnctrl_ctx, nnctrl_ctx->verbose, nnctrl_ctx->cache_en, nnctrl_ctx->reuse_mem);
    rval = load_net(nnctrl_ctx);

    if (rval < 0) {
        printf("init net context, return %d\n", rval);
    }
    return rval;
}

int cnn_run(nnctrl_ctx_t *nnctrl_ctx, float *output[], int output_count)
{
    int rval = 0;
    
    rval = nnctrl_run_net(nnctrl_ctx->net.net_id, &nnctrl_ctx->net.result, NULL, NULL, NULL);

    if (rval < 0)
    {
        DPRINT_ERROR("nnctrl_run_net() failed, return %d\n", rval);
    }

    // parse the output of net
    if (nnctrl_ctx->cache_en) {
        cavalry_sync_cache(nnctrl_ctx->net.net_m.mem_size, nnctrl_ctx->net.net_m.phy_addr, 0, 1);
    }
    
    for (int i = 0; i < output_count; i++)
	{
        float *score_addr = (float *)(nnctrl_ctx->net.net_m.virt_addr
                            + nnctrl_ctx->net.net_out.out_desc[i].addr - nnctrl_ctx->net.net_m.phy_addr);

		output[i] = score_addr;	
    }

    return rval;
}
