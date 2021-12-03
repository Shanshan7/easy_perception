#ifndef CNN_DATA_STRUCTURE_H
#define CNN_DATA_STRUCTURE_H

#include <cavalry_ioctl.h>
#include <cavalry_mem.h>
#include <nnctrl.h>

#define MAX_FILE_NAME_LEN			(256)

typedef struct cavalry_ctx_s {
    int fd_cavalry;
} cavalry_ctx_t;

struct net_match {
	uint8_t net_id;

	struct net_run_cfg net_rev;
	struct net_mem net_m;

    struct net_cfg cfg;
    struct net_input_cfg net_in;
    struct net_output_cfg net_out;
    struct net_result result;

    char net_file[MAX_FILE_NAME_LEN];
};

typedef struct nnctrl_ctx_s {
    uint8_t verbose;
    uint8_t reuse_mem;
    uint8_t cache_en;
    uint8_t buffer_id;
    uint8_t log_level;

    struct net_match net;
} nnctrl_ctx_t;


#endif //CNN_DATA_STRUCTURE_H