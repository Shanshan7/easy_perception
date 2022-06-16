/*************************************************************************************
 Copyright (c) 2018-2022, Unisinsight Technologies Co., Ltd. All rights reserved.
-------------------------------------------------------------------------------------
                            cavalry_gen.h
   Project Code: cavalry_gen数据类型定义
   Module Name :
   Date Created: 2021-2-4
   Author      :  
   Description : cavalry_gen数据类型定义
**************************************************************************************/
#ifndef __CAVALRY_GEN_H__
#define __CAVALRY_GEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CAVALRY_GEN_VERSION		(0x02020008)
#define CAVALRY_COMPAT_VERSION	(0x00000002)
#define CAVALRY_VAR_NAME_MAX		(128)
#define CAVALRY_IO_NAME_MAX		(512)
#define CAVALRY_IO_DEMNGL_NAME_MAX		(512)
#define CAVALRY_IO_PARENT_NAME_MAX		(64)
#define CAVALRY_VPROC_VAR_NAME_MAX		(32)
#define CAVALRY_VPROC_SMB_NAME_MAX		(16)

/* cavalry_gen will set invalid value if not found data_format */
typedef enum datasize_type_s {
	DATASIZE_8_BITS = 0,
	DATASIZE_16_BITS = 1,
	DATASIZE_32_BITS = 2,
	DATASIZE_64_BITS = 3,
	DATASIZE_INVALID = 0xFF,
} datasize_type_t;

typedef enum {
	CAVALRY_BIN_UNENCRYPTED = 0,
	CAVALRY_BIN_ENCRYPTED_BY_PEK = 1,
	CAVALRY_BIN_ENCRYPTED_BY_DEK = 2,  /* deprecated */
} cavalry_bin_state;

typedef enum {
	CAVALRY_VDG_ARCH_BYPASS = 0x0,
	CAVALRY_VDG_ARCH_CV22 = 0x1,
	CAVALRY_VDG_ARCH_CV2 = 0x2,
	CAVALRY_VDG_ARCH_CV25 = 0x3,
	CAVALRY_VDG_ARCH_CV28 = 0x4,
	CAVALRY_VDG_ARCH_CV5 = 0x5,
} cavalry_vdg_arch_t;

/* header info */
typedef struct cavalry_gen_header_s {
	uint32_t version_info;
	uint32_t ver_compat;
	uint32_t version_hash;
	uint32_t dvi_num;
	uint32_t dvi_desc_size;
	uint32_t io_desc_size;
	uint32_t vproc_desc_size;
	uint32_t func_var_size;
	uint32_t smb_desc_size;
	uint32_t vproc_info_offset;
	uint32_t usr_info_offset;
	uint32_t usr_info_size;
	uint32_t checksum;
	uint32_t encrypt_state: 4;
	uint32_t reserve0: 4;
	uint32_t vdg_arch: 8;
	uint32_t reserve1: 16;
	uint32_t encrypt_digest[8];

	uint32_t json_file_offset;
	uint32_t json_file_size;
	uint32_t graph_inner_loop_num;
	uint32_t graph_inner_loop_desc_size;
	uint32_t loop_dag_desc_size;
	uint32_t port_pair_desc_size;
	uint32_t graph_joint_num;
	uint32_t graph_joint_desc_size;

	uint32_t reserve2[98];
} cavalry_gen_header_t;

/*per dvi descriptor */
typedef struct dvi_desc_s {
	uint32_t dvi_id;
	uint32_t graph_id;
	uint32_t vproc_id;
	uint32_t dvi_ppv: 1;
	uint32_t reserve_dvi: 31;
	uint32_t dvi_img_vaddr;
	uint32_t dvi_img_size;
	uint32_t dvi_dag_vaddr;
	uint32_t input_num;
	uint32_t output_num;
	uint32_t dvi_pkg_size;
	char dag_name[CAVALRY_VAR_NAME_MAX];
} dvi_desc_t;

/*per port descriptor (HMB_input HMB_output) */
typedef struct io_descriptor_s{
	uint32_t port_dim_p;
	uint32_t port_dim_d;
	uint32_t port_dim_h;
	uint32_t port_dim_w;
	uint32_t port_pitch;		/* dpitch_num_bytes */
	uint32_t port_pitch_offset;

	/* 1-bit variable only */
	uint32_t port_drotate: 1;
	uint32_t port_hflip: 1;
	uint32_t port_vflip: 1;
	uint32_t port_dflip: 1;
	uint32_t port_pflip: 1;
	uint32_t port_is_main_io: 1;
	uint32_t port_dim_bitvector: 1;
	uint32_t port_scalar_init : 1;
	uint32_t port_scalar_variable : 1;
	uint32_t reserve_port: 23;

	uint8_t port_dram_format;
	uint8_t port_pitch_bsize;
	uint8_t port_slice_total_num;
	uint8_t port_slice_seq;
	uint32_t port_slice_byte_offset;
	char port_slice_parent_name[CAVALRY_IO_PARENT_NAME_MAX];

	uint32_t port_drotate_bit_offset;
	uint32_t port_hflip_bit_offset;
	uint32_t port_vflip_bit_offset;
	uint32_t port_dflip_bit_offset;
	uint32_t port_pflip_bit_offset;

	uint8_t port_data_sign;	/*data format: sign, datasize, expoffset, expbits*/
	uint8_t port_data_size;
	int8_t port_data_expoffset;
	uint8_t port_data_expbits;

	uint32_t port_scalar_var_default;

	uint32_t port_size;
	uint32_t port_byte_offset;	/* dbase_byte_offset */
	char port_name[CAVALRY_IO_NAME_MAX];
	char port_demangled_name[CAVALRY_IO_DEMNGL_NAME_MAX];
} io_descriptor_t;

/* vproc descriptors */
typedef struct vproc_desc_s {
	uint32_t var_num;
	uint32_t smb_num;
	uint32_t vproc_pkg_size;
}vproc_desc_t;

/* functional parameters */
typedef struct func_variable_s{
	char var_name[CAVALRY_VPROC_VAR_NAME_MAX];
	uint32_t var_boffset;
	uint32_t var_bsize;
} func_variable_t;

/* SMB descriptor */
typedef struct smb_descriptor_s{
	char smb_name[CAVALRY_VPROC_SMB_NAME_MAX];
	uint32_t vbase_byte_offset;
} smb_descriptor_t;

/* Section for graph internal loop and graph joint - START */

typedef struct graph_inner_loop_s {
	uint32_t graph_id;
	char const_vect_name[CAVALRY_IO_NAME_MAX];
	uint32_t const_vect_size;
	uint32_t loop_cnt;
	uint32_t loop_dag_cnt;
	uint32_t port_pair_cnt;
} graph_inner_loop_t;

typedef struct graph_inner_loop_dag_s {
	uint32_t dvi_id;
} graph_inner_loop_dag_t;

typedef struct graph_inner_port_pair_s {
	char port_name[CAVALRY_IO_NAME_MAX];
	char src_port_name[CAVALRY_IO_NAME_MAX];
	uint32_t init_vect_size;
	uint8_t init_vect_value[0];	 // DON'T add item after init_vect_value[0]
} graph_inner_port_pair_t;

typedef struct graph_joint_s {
	uint32_t dvi_id;
	uint32_t graph_id;
	char port_name[CAVALRY_IO_NAME_MAX];

	uint32_t src_dvi_id;
	uint32_t src_graph_id;
	char src_port_name[CAVALRY_IO_NAME_MAX];
} graph_joint_t;

/* Section for graph internal loop and graph joint - END */


#ifdef __cplusplus
}
#endif

#endif

