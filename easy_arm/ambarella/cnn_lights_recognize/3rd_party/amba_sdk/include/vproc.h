/*************************************************************************************
 Copyright (c) 2018-2022, Unisinsight Technologies Co., Ltd. All rights reserved.
-------------------------------------------------------------------------------------
                            vproc.h
   Project Code: vproc数据类型定义
   Module Name :
   Date Created: 2021-2-4
   Author      :  
   Description : vproc数据类型定义
**************************************************************************************/
#ifndef _VPROC_H_
#define _VPROC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IN
#define OUT
#define IN_OUT

/*! @addtogroup vproc-helper
 * @{
 */

/*!
 * @brief The version of vproc library
 */
struct vproc_version {
	uint32_t major;  /*!< Version major number */
	uint32_t minor;  /*!< Version minor number */
	uint32_t patch;  /*!< Version patch number */
	unsigned int mod_time;  /*!< Version modification time */
	char description[64];  /*!< Version description */
};

/*!
 * @brief The data type
 */
typedef enum data_type_s {
	DT_FX8 = 0,  /*!< fixed 8-bit data */
	DT_FX16 = 1, /*!< fixed 16-bit data */
	DT_FP16 = 2, /*!< float 16-bit data */
	DT_FP32 = 3, /*!< float 32-bit data */
} data_type_t;

/*!
 * @brief The color space for vector
 */
typedef enum color_space_s {
	CS_VECT = 0,  /*!< General vector */

	/* RGB list*/
	CS_RGB = 1, /*!< AMB RGB format, refering to plannar RGB data */
	CS_BGR = 2, /*!< AMB BGR format, refering to plannar BGR data */
	CS_RGB_ITL = 3, /*!< OpenCV RGB format, refering to element-interleaved RGB data */
	CS_BGR_ITL = 4, /*!< OpenCV BGR format, refering to element-interleaved BGR data */
	CS_RGB_LAST,
	CS_RGB_FIRST = CS_RGB,

	/* YUV list*/
	CS_NV12 = 10,	/*!< YUV NV12 format */
	CS_Y = 11,	/*!< gray/single channel format */
	CS_ITL = 12,	/*!< element-interleaved UV data */
	CS_YUV_LAST,
	CS_YUV_FIRST = CS_NV12,
} color_space_t;

/*!
 * @brief The flags of statistics methods
 */
typedef enum stats_flag_s {
	STATS_CALC_MIN = (1<<0), /*!< the flag of enabling minimum calculation */
	STATS_CALC_MAX = (1<<1), /*!< the flag of enabling maximum calculation */
	STATS_CALC_AVG = (1<<2), /*!< the flag of enabling mean calculation */
	STATS_CALC_STD = (1<<3), /*!< the flag of enabling standard deviation calculation */
	STATS_CALC_ALL = STATS_CALC_MIN | STATS_CALC_MAX
		| STATS_CALC_AVG | STATS_CALC_STD, /*!< the flag of enabling all calculation */
} stats_flag_t;

/*!
 * @brief The flags of resize methods
 */
typedef enum resize_flag_s {
	RESIZE_MUL_STEPS = 0, /*!< the flag of enabling multi-step resize strategy for downscale */
	RESIZE_SIN_STEP = 1, /*!< the flag of enabling one-step resize strategy for downscale */
} resize_flag_t;

/*!
 * @brief The flags of transpose operation
 */
typedef enum transpose_flag_s {
	TRANSPOSE_CHW2HWC = 0, /*!< the flag of doing CHW2HWC transposition */
	TRANSPOSE_HWC2CHW = 1, /*!< the flag of doing HWC2CHW transposition */
	TRANSPOSE_LAST,
	TRANSPOSE_FIRST = TRANSPOSE_CHW2HWC,
} transpose_flag_t;


/*!
 * @brief The output pattern converted from bayer pattern
 */
#define BAYER_CVT_RGB	0
#define BAYER_CVT_BGR	1

/*!
 * @brief compare mode of black/white binarization
*/
#define  BW_GREATER_THAN 0
#define  BW_LOWER_THAN 1
#define  BW_GREATER_EQUAL_THAN 2
#define  BW_LOWER_EQUAL_THAN 3
#define  BW_EQUAL_TO 4
#define  BW_NOT_EQUAL_TO 5


/*!
 * @brief The customized error code
 */
#define WARP_TABLE_OVERFLOW 0x0FFF0000 /*!< the error code indicates the warp table is overflowed */

/*!
 * @brief The ROI area on a vector
 */
typedef struct roi_area_s {
	uint32_t xoffset;  /*!< starting ROI position at X-axis */
	uint32_t yoffset;  /*!< starting ROI position at Y-axis */
	uint32_t width;  /*!< width of ROI */
	uint32_t height;  /*!< height of ROI */
} roi_area_t;

/*!
 * @brief The shape of a vector
 */
typedef struct vect_shape_s {
	uint32_t p;  /*!< the plane of a vector */
	uint32_t d;  /*!< the depth of a vector */
	uint32_t h;  /*!< the height of a vector */
	uint32_t w;  /*!< the width of a vector */
} vect_shape_t;

/*!
 * @brief The data format representation
 */
typedef struct data_format_s{
	int8_t sign;  /*!< sign bit: 0 for unsigned, 1 for signed */
	int8_t datasize;  /*!< datasize bit: 0 for 8bit, 1 for 16bit */
	int8_t exp_offset;  /*!< Q value for quantized data */
	int8_t exp_bits;  /*!< exp bits for floating point data */
} data_format_t;

/*!
 * @brief The vector description
 */
typedef struct vect_desc_s {
	uint32_t data_addr; /*!< the start dram address that keeps the vector data */
	uint32_t pitch; /*!< the pitch of the vector */
	vect_shape_t shape; /*!< the shape of the vector */
	data_format_t data_format; /*!< the data format of the vector */
	color_space_t color_space; /*!< the color space of the vector */
	roi_area_t roi; /*!< the ROI in the vector */
	uint32_t reserved[8]; /*!< reserved field */
} vect_desc_t;

/*!
 * @brief The vector description with memory fd
 */
typedef struct vect_desc_mfd_s {
	int data_addr_fd; /*!< the memory fd that allocated for vector data */
	uint32_t data_addr_offset; /*!< the address offset based on memory fd where keeps vector data */
	uint32_t pitch; /*!< the pitch of the vector */
	vect_shape_t shape; /*!< the shape of the vector */
	data_format_t data_format; /*!< the data format of the vector */
	color_space_t color_space; /*!< the color space of the vector */
	roi_area_t roi; /*!< the ROI of the vector */
	uint32_t reserved[8]; /*!< reserved field */
} vect_desc_mfd_t;

/*!
 * @brief The extra information required by vproc_image_deformation
 */
typedef struct deformation_extra_s {
	int32_t uv_offset; /*!< the address offset based on Y data that keeps UV data */
	uint32_t reserved[8]; /*!< reserved field */
} deformation_extra_t;

/*!
 * @brief The extra information required by vproc_image_polish
 */
typedef struct polish_extra_s {
	vect_desc_t *mean; /*!< the vector keeps mean values */
	float scale; /*!< the scale factor */
	uint32_t reserved[8]; /*!< reserved field */
} polish_extra_t;

/*!
 * @brief The extra information required by vproc_image_polish_mfd
 */
typedef struct polish_extra_mfd_s {
	vect_desc_mfd_t *mean; /*!< the vector keeps mean values */
	float scale; /*!< the scale factor */
	uint32_t reserved[8]; /*!< reserved field */
} polish_extra_mfd_t;

/*!
 * @brief The yuv2rgb matrix parameters. The yuv2rgb_mat is defined as below
	 [R G B]T = [yc 0 rv; yc -gu -gv; yc bu 0] x [Y-yb U-128 V-128]T
 */
typedef struct yuv2rgb_mat_s {
	float yc; /*!< default value is 1 */
	float rv; /*!< default value is 1.402 */
	float gu; /*!< default value is 0.344136 */
	float gv; /*!< default value is 0.714136 */
	float bu; /*!< default value is 1.772 */
	float yb; /*!< default value is 0 */
	uint32_t reserved[6]; /*!< reserved field */
} yuv2rgb_mat_t;

/*!
 * @brief The rgb2yuv matrix parameters. The rgb2yuv_mat is defined as below
	[Y - bias[0], U - bias[1], V - bias[2]]T
	= [matrtix[0], matrtix[1], matrtix[2];
	   matrtix[3], matrtix[4], matrtix[5];
	   matrtix[6], matrtix[7], matrtix[8]] x [R, G, B]T
 */
typedef struct rgb2yuv_mat_s {
	float matrix[9]; /*!< default value is 0.257,0.504,0.098,-0.148,-0.291,0.439,0.439,-0.368,-0.071 */
	uint8_t bias[3]; /*!< default value is 16, 128, 128 */
	uint8_t reserved0; /*!< reserved field */
	uint32_t reserved1[6]; /*!< reserved field */
} rgb2yuv_mat_t;

/*!
 * @brief The configuration of resize
 */
typedef struct resize_cfg_s {
	resize_flag_t method; /*!< resize method */

	uint32_t batch_num: 16; /*!< the batch number */
	uint32_t reserved0: 16; /*!< reserved field */
	uint32_t reserved[8]; /*!< reserved field */
} resize_cfg_t;

/*!
 * @brief The configuration of Harris point detection
 */
typedef struct harris_cfg_s {
	float threshold; /*!< The Harris response threshold */
	uint32_t nms_size; /*!< The Harris NMS window size */
	uint32_t reserved[128]; /*!< reserved field */
} harris_cfg_t;

/*!
 * @brief The configuration of LK optical flow algorithm
 */
typedef struct optlk_cfg_s {
	float resp_threshold; /*!< the threshold of the LK response,
		* carry out LK calculation when the response above the threshold,
		* otherwise dx/dy set to zero */
	int32_t with_k; /*!< non-zero: k=0.04, zero: k=0 */
	uint32_t reserved[8]; /*!< reserved field */
} optlk_cfg_t;

/*!
 * @brief The general configuration of kernel
 */
typedef struct cvker_s{
	float* ker_content; /*!< The content of kernel */
	uint32_t ker_h; /*!<  kernel height */
	uint32_t ker_w; /*!<  kernel width */
	void* mem_virt; /*!<  kernel memory virtual address */
	unsigned long mem_size; /*!<  kernel memory size */
	uint32_t mem_phy;  /*!<  kernel memory physical address */
	uint32_t reserved[8]; /*!< reserved field */
}cvker_t;

/*!
 * @brief The general configuration of kernel in mfd
 */
typedef struct cvker_mfd_s{
	float* ker_content; /*!< The content of kernel */
	uint32_t ker_h; /*!<  kernel height */
	uint32_t ker_w; /*!<  kernel width */
	void* mem_virt;/*!<  kernel memory virtual address */
	unsigned long mem_size; /*!<  kernel memory size */
	int mem_mfd; /*!<  kernel memory mfd */
	uint32_t mem_addr_offset; /*!<  kernel memory address offset */
	uint32_t reserved[8]; /*!< reserved field */
}cvker_mfd_t;


/*!
 * @brief The configuration of image binarization
 */
typedef struct bw_cfg_s {
	int32_t with_abs; /*!< do abs() before applying the threshold if set. */
	float bw_threshold; /*!< black/white threshold */
	uint32_t compare_method; /*!< 0:>  1:<  2:>=  3:<=  4:==  5:!= */
	uint8_t black_value; /*!< black value from 0 to 255 */
	uint8_t white_value; /*!< white value from 0 to 255 */
	uint32_t reserved[8]; /*!< reserved field */
} bw_cfg_t;


/*!
 * @brief The configuration of bayer pattern to BGR
 */
typedef struct bayer2bgr_cfg_s {
	uint32_t height; /*!< input height */
	uint32_t width; /*! < input width */
	uint32_t out_pattern:16; /*!< BAYER_CVT_RGB: bayer2rgb, BAYER_CVT_BGR:bayer2bgr */
	uint32_t rsvd:16;  /*!< reserved field */
	uint32_t reserved[8]; /*!< reserved field */
} bayer2bgr_cfg_t;


/*!
 * @brief The configuration of image binarization
 */
typedef struct cclb_cfg_s {
	uint8_t bw_threshold; /*!< black/white threshold */
	uint8_t cctype; /*!< 0:4-way connectivity,1:8-way connectivity */
	uint16_t min_comp_height;	/*!< minimal height of component */
	uint16_t min_comp_width; /*!< minimal width of component */
	uint16_t min_comp_pcount; /*!< minimal points count of component */
	uint16_t min_hole_height;	/*!< minimal height of hole */
	uint16_t min_hole_width; /*!< minimal width of hole */
	uint16_t min_hole_pcount; /*!< minimal hole count of component */
	uint16_t compare_method; /*!< BW_GREATER_THAN,  BW_LOWER_THAN,  BW_GREATER_EQUAL_THAN,  BW_LOWER_EQUAL_THAN,  BW_EQUAL_TO,  BW_NOT_EQUAL_TO */
	uint32_t reserved[8]; /*!< reserved field */
} cclb_cfg_t;

/*!
 * @brief The configuration of image binarization
 */
typedef struct cclb_out_s {
	uint16_t row; /*!< start row */
	uint16_t col; /*!start column */
	uint16_t length; /*!< length of segment */
	uint16_t label;	/*!< label value */
} cclb_out_t;

/*!
 * @brief The configuration of warp affine transformation
 */
typedef struct warpaffine_cfg_s {
	float *mat; /*!< the transformation matrix */
	void *wfld_mem_virt; /*!< the virtual address of allocated memory where warp field data is kept */
	uint32_t wfld_mem_phys; /*!< the physical address of allocated memory where warp field data is kept */
	uint32_t wfld_mem_cache_en: 1; /*!< the flag to indicate if the allocated memory for warp field is cached */
	uint32_t inverse_mat: 1; /*!< the flag to indicate if the provided transformation matrix needs reversion */
	uint32_t reserved_0: 30; /*!< reserved field */
	uint32_t reserved[8]; /*!< reserved field */
} warpaffine_cfg_t;

/*!
 * @brief The configuration of warp affine transformation in mfd mode
 */
typedef struct warpaffine_cfg_mfd_s {
	float *mat; /*!< the transformation matrix */
	void *wfld_mem_virt; /*!< the virtual address of allocated memory where warp field data is kept */
	int wfld_mem_fd; /*!< the memory fd of allocated memory where warp field data is kept */
	uint32_t wfld_mem_offset; /*!< the physical address offset of allocated memory where warp field data is kept */
	uint32_t wfld_mem_cache_en: 1; /*!< the flag to indicate if the allocated memory for warp field is cached */
	uint32_t inverse_mat: 1; /*!< the flag to indicate if the provided transformation matrix needs reversion */
	uint32_t reserved_0: 30; /*!< reserved field */
	uint32_t reserved[8]; /*!< reserved field */
} warpaffine_cfg_mfd_t;

/*!
 * @brief The configuration of warp perspective transformation
 */
typedef struct warppersp_cfg_s {
	float *mat;  /*!< the transformation matrix */
	void *mat_mem_virt; /*!< the virtual address of allocated memory for transformation matrix */
	uint32_t mat_mem_phys; /*!< the physical address of allocated memory for transformation matrix */
	uint32_t mat_mem_cache_en: 1; /*!< the flag to indicate if the allocated memory for transformation matrix is cached */
	uint32_t inverse_mat: 1; /*!< the flag to indicate if the provided transformation matrix needs reversion */
	uint32_t reserved_0: 30; /*!< reserved field */
	uint32_t reserved[8]; /*!< reserved field */
} warppersp_cfg_t;

/*!
 * @brief The configuration of warp perspective transformation
 */
typedef struct warppersp_cfg_mfd_s {
	float *mat;  /*!< the transformation matrix */
	void *mat_mem_virt; /*!< the virtual address of allocated memory for transformation matrix */
	int mat_mem_fd; /*!< the memory fd of allocated memory for transformation matrix */
	uint32_t mat_mem_offset; /*!< the physical address offset of allocated memory for transformation matrix */
	uint32_t mat_mem_cache_en: 1; /*!< the flag to indicate if the allocated memory for transformation matrix is cached */
	uint32_t inverse_mat: 1; /*!< the flag to indicate if the provided transformation matrix needs reversion */
	uint32_t reserved_0: 30; /*!< reserved field */
	uint32_t reserved[8]; /*!< reserved field */
} warppersp_cfg_mfd_t;

/*! @macros AMBA_API
 *  @brief API function attribute */
#ifndef AMBA_API
#define AMBA_API __attribute__((visibility("default")))
#endif
/*! @} */ /* End of vproc-helper */

/*!
 * @addtogroup vproc-api-details
 * @{
 */

/*!
 * This API get the version of library.
 *
 * @param ver the pointer to version
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_get_version(struct vproc_version *ver);

/*!
 * This API initializes the library and parses the DAGs information for
 * the library. It must be called before all other functions. It will return
 * the library size that should be allocated by the APP.
 *
 * @param bin_name VProc library binary generated by cavalry_gen.
 * @param size The total memory required to store the DVI DAGs.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_init(IN const char *bin_name,
	IN_OUT uint32_t *size);

/*!
 * This API initializes the library and parses the DAGs information for
 * the library from memory. It must be called before all other functions.
 * It will return the library size that should be allocated by the APP.
 *
 * @param feed_virt Virtual address that keeps VProc library data.
 * @param size The total memory required to store the DVI DAGs.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_init_from_mem(IN void *feed_virt,
	IN_OUT uint32_t *size);

/*!
 * This API initializes the Cavalry driver and loads the DVI required by
 * the VProc library to the specified memory address.
 *
 * @param fd_cavalry FD of /dev/cavalry.
 * @param virt_addr User space address of the CV user memory allocated by the APP.
 * @param phy_addr User space physical address of the CV user memory allocated by the APP.
 * @param size CV user memory size for CV allocated by the APP.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_load(IN int fd_cavalry,
	IN uint8_t *virt_addr,
	IN uint32_t phy_addr,
	IN uint32_t size);

/*!
 * This API initializes the Cavalry driver and loads the DVI required by
 * the VProc library to the specified memory address with memory fd.
 *
 * @param fd_cavalry FD of /dev/cavalry.
 * @param virt_addr User space address offset of the CV user memory allocated by the APP.
 * @param lib_mfd User space fd of the CV user memory allocated by the APP.
 * @param size CV user memory size for CV allocated by the APP.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_load_mfd(IN int fd_cavalry,
	IN uint8_t *virt_addr,
	IN int lib_mfd,
	IN uint32_t size);

/*!
 * This API exit the library.
 * It must be called after all other functions.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_exit();

/*!
 * This API sets the library executional priority.
 * It could be called before targeted VProc API.
 * @param pid The pid that needs to set priority.
 * @param priority The target priority.
 * @return 0 = success, -1 = error.
 */
AMBA_API int set_vproc_priority(int pid,
	uint8_t priority);

/*!
 * This API crops images, or data as vectors where cropping area is the ROI
 * in input vector.
 *
 * @param src Input vector data descriptor
 * @param dst Input vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_crop(IN vect_desc_t *src,
	OUT vect_desc_t *dst);

/*!
 * This API crops images, or data as vectors where cropping area is the ROI
 * in input vector. The data is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Input vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_crop_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst);

/*!
 * This API resizes data in groups. The resize operations ignore the data format
 * setting in the SRC and DST vectors. These vectors are taken in the
 * data format (0,0,0,0).
 *
 * @param src_vect A group of input vector data descriptors
 * @param dst_vect A group of output vector data descriptors
 * @param batch_num Number of elements in the group.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_vect_batch(vect_desc_t *src_vect,
	vect_desc_t *dst_vect, int count);

/*!
 * This API resizes data in groups. The resize operations ignore the data format
 * setting in the SRC and DST vectors. These vectors are taken in the
 * data format (0,0,0,0). The data is stored on the allocated memory with mfd.
 *
 * @param src_vect A group of input vector data descriptors
 * @param dst_vect A group of output vector data descriptors
 * @param batch_num Number of elements in the group.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_vect_batch_mfd(vect_desc_mfd_t *src_vect,
	vect_desc_mfd_t *dst_vect, int count);

/*!
 * This API resizes the YUV, RGB images, or data as vectors. All data are stored
 * continuously on memory for every channel data address, as calculated by
 * the given start address. It ignores the data format setting in SRC and DST
 * vectors, and takes these vectors in the data format (0, 0, 0, 0).
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize(IN vect_desc_t *src,
	OUT vect_desc_t *dst);

/*!
 * This API resizes the YUV, RGB images, or data as vectors. All data are stored
 * continuously on memory for every channel data address, as calculated by
 * the given start address. It ignores the data format setting in SRC and DST
 * vectors, and takes these vectors in the data format (0, 0, 0, 0). The data
 * is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst);

/*!
 * This API resizes data by taking the provided configuration into account,
 * such as resize method, batch mode and so on. The resize operations ignore
 * the data format setting in the SRC and DST vectors. These vectors are taken
 * in the data format (0,0,0,0).
 *
 * @param src A group of input vector data descriptors
 * @param dst A group of output vector data descriptors
 * @param cfg Resize configuration
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_ext(IN vect_desc_t *src,
	OUT vect_desc_t *dst, IN resize_cfg_t *cfg);

/*!
 * This API resizes data by taking the provided configuration into account,
 * such as resize method, batch mode and so on. The resize operations ignore
 * the data format setting in the SRC and DST vectors. These vectors are taken
 * in the data format (0,0,0,0). The data is stored on the allocated memory
 * with mfd.
 *
 * @param src A group of input vector data descriptors
 * @param dst A group of output vector data descriptors
 * @param cfg Resize configuration
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_ext_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst, IN resize_cfg_t *cfg);

/*!
 * This API resizes data in a single channel. Support one channel data which
 * could be Y-Channel, R-Channel, G-Channel, B-Channel and so on. Resize
 * operations ignore the data format setting in the SRC and DST vectors. These
 * vectors are taken in the data format (0,0,0,0).
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_single_channel(IN vect_desc_t *src,
	OUT vect_desc_t *dst);

/*!
 * This API resizes data in a single channel. Support one channel data which
 * could be Y-Channel, R-Channel, G-Channel, B-Channel and so on. Resize
 * operations ignore the data format setting in the SRC and DST vectors. These
 * vectors are taken in the data format (0,0,0,0). The data is stored on the
 * allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_single_channel_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst);

/*!
 * Resizes data in the interleaved format.upport 2-channel interleaved whick
 * looks like UVUVUVUVUVUV. Resize operations ignore the data format setting in
 * the SRC and DST vectors. These vectors are taken in the data format
 * (0,0,0,0).
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_interleave_channel(IN vect_desc_t *src,
	OUT vect_desc_t *dst);

/*!
 * Resizes data in the interleaved format.upport 2-channel interleaved whick
 * looks like UVUVUVUVUVUV. Resize operations ignore the data format setting in
 * the SRC and DST vectors. These vectors are taken in the data format
 * (0,0,0,0). The data is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_resize_interleave_channel_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst);

/*!
 * This API converts the YUV (420) format to RGB. The color space of output is
 * supposed to be plannar RGB or BGR
 *
 * @param y Y channel vector data descriptor
 * @param uv UV channel vector data descriptor
 * @param rgb Output RGB vector data descriptor
 * @param yuv2rgb_mat Matrix data to convert YUV to RGB
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_yuv2rgb_420(IN vect_desc_t *y,
	IN vect_desc_t *uv,
	OUT vect_desc_t *rgb,
	IN yuv2rgb_mat_t *yuv2rgb_mat);

/*!
 * This API converts the YUV (420) format to RGB. The color space of output is
 * supposed to be plannar RGB or BGR. The data is stored on the allocated
 * memory with mfd.
 *
 * @param y Y channel vector data descriptor
 * @param uv UV channel vector data descriptor
 * @param rgb Output RGB vector data descriptor
 * @param yuv2rgb_mat Matrix data to convert YUV to RGB
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_yuv2rgb_420_mfd(IN vect_desc_mfd_t *y,
	IN vect_desc_mfd_t *uv,
	OUT vect_desc_mfd_t *rgb,
	IN yuv2rgb_mat_t *yuv2rgb_mat);

/*!
 * This API substracts the mean value from source vector.
 *
 * @param src Input vector data descriptor
 * @param mean Mean value vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_submean(IN vect_desc_t *src,
	IN vect_desc_t *mean,
	OUT vect_desc_t *dst);

/*!
 * This API substracts the mean value from source vector. The data is stored on
 * the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param mean Mean value vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_submean_mfd(IN vect_desc_mfd_t *src,
	IN vect_desc_mfd_t *mean,
	OUT vect_desc_mfd_t *dst);

/*!
 * This API scales the data by multiplying it with a scaling factor. The result
 * can be saved to FX8/FX16/FP16/FP32.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param scl Scaling factor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_scale_ext(IN vect_desc_t *src,
	OUT vect_desc_t *dst,
	IN float scl);

/*!
 * This API scales the data by multiplying it with a scaling factor. The result
 * can be saved to FX8/FX16/FP16/FP32. The data is stored on the allocated
 * memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param scl Scaling factor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_scale_ext_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst,
	IN float scl);

/*!
 * This API converts the data type to Fixed 8-bit, Fixed 16-bit,
 * Float 16-bit and Float 32-bit.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_dtcvt(IN vect_desc_t *src,
	OUT vect_desc_t *dst);

/*!
 * This API converts the data type to Fixed 8-bit, Fixed 16-bit,
 * Float 16-bit and Float 32-bit. The data is stored on the allocated
 * memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_dtcvt_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst);

/*!
 * This API rotates or flips data using the given rotation-flip bitmap that
 * comes with the FX8 data format.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param src_rotate_flip_bitmap Input rotate_flip_bitmap.
 * 4: pflip; 3: dflip; 2: vflip; 1: hflip; 0: rotate. E.g. vflip+rotate: 0x05 (00101)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_rotate(IN vect_desc_t *src,
	OUT vect_desc_t *dst, IN uint32_t src_rotate_flip_bitmap);

/*!
 * This API rotates or flips data using the given rotation-flip bitmap that
 * comes with the FX8 data format. The data is stored on the allocated
 * memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param src_rotate_flip_bitmap Input rotate_flip_bitmap.
 * 4: pflip; 3: dflip; 2: vflip; 1: hflip; 0: rotate. E.g. vflip+rotate: 0x05 (00101)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_rotate_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst, IN uint32_t src_rotate_flip_bitmap);

/*!
 * This API converts between the Amberalla RGB image format, which is
 * plane-interleaved, and the OpenCV RGB image format, which is
 * element-interleaved.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_imcvt(IN vect_desc_t *src,
	OUT vect_desc_t *dst);

/*!
 * This API converts between the Amberalla RGB image format, which is
 * plane-interleaved, and the OpenCV RGB image format, which is
 * element-interleaved. The data is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_imcvt_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst);

/*!
 * This API converts the 10bitpacked data format to UInt16 by padding 6 MSB
 * for each element.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_10bitpacked_to_uint16(IN vect_desc_t *src,
	OUT vect_desc_t *dst);

/*!
 * This API converts the 10bitpacked data format to UInt16 by padding 6 MSB
 * for each element. The data is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_10bitpacked_to_uint16_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst);

/*!
 * A high-level API handles image deformation operations, such as yuv2rgb,
 * resize which internally calls low-level APIs. If yuv2rgb conversion is
 * applied, the color space of output is supposed to be plannar RGB or BGR.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param dext Extra data required for operations, such as the UV data offset from the Y data address
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_image_deformation(IN vect_desc_t *src,
	OUT vect_desc_t *dst, IN deformation_extra_t *dext);

/*!
 * A high-level API handles image deformation operations, such as yuv2rgb,
 * resize which internally calls low-level APIs.  If yuv2rgb conversion is
 * applied, the color space of output is supposed to be plannar RGB or BGR.
 * The data is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param dext Extra data required for operations, such as the UV data offset from the Y data address
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_image_deformation_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst, IN deformation_extra_t *dext);

/*!
 * A high-level API handles data pre-processing operations, such as
 * mean subtraction, scale, data format conversion, which internally
 * calls low-level APIs.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param pext Extra data required for operations, such as mean values and scaling factor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_data_polish(IN vect_desc_t *src,
	OUT vect_desc_t *dst, IN polish_extra_t *pext);

/*!
 * A high-level API handles data pre-processing operations, such as
 * mean subtraction, scale, data format conversion, which internally
 * calls low-level APIs. The data is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param pext Extra data required for operations, such as mean values and scaling factor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_data_polish_mfd(IN vect_desc_mfd_t *src,
	OUT vect_desc_mfd_t *dst, IN polish_extra_mfd_t *pext);

/*!
 * This API performs the Harris Points detection.
 *
 * @param im_vect Input vector data of the image
 * @param threshold Input Harris response threshold
 * @param det_bit_vect Output bit vector of Harris Points detection
 * @param resp_vect Output data vector of Harris response
 * @param maxresp_vect Output peak value of Harris response
 * @return 0 = success, -1 = error.
 *
 * The following shows the calculation of harris points detection
 * @image html harris_calculation.jpg "calculation of harris points detection"
 */
AMBA_API int vproc_harris(IN vect_desc_t *im_vect, IN float threshold,
	OUT vect_desc_t *det_bit_vect, OUT vect_desc_t *resp_vect, OUT vect_desc_t *maxresp_vect);

/*!
 * This API performs the Harris Points detection. The data is stored on the
 * allocated memory with mfd.
 *
 * @param im_vect Input vector data of the image
 * @param threshold Input Harris response threshold
 * @param det_bit_vect Output bit vector of Harris Points detection
 * @param resp_vect Output data vector of Harris response
 * @param maxresp_vect Output peak value of Harris response
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_harris_mfd(IN vect_desc_mfd_t *im_vect, IN float threshold,
	OUT vect_desc_mfd_t *det_bit_vect, OUT vect_desc_mfd_t *resp_vect, OUT vect_desc_mfd_t *maxresp_vect);

/*!
 * This API performs Harris Point detection with an extended function
 * (supporting the NMS window size adaption).
 *
 * @param im_vect Input vector data of the image
 * @param harris_param Input harris response threshold, nms window size
 * @param det_bit_vect Output bit vector of Harris Points detection
 * @param resp_vect Output data vector of Harris response
 * @param maxresp_vect Output peak value of Harris response
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_harris_ext(IN vect_desc_t *im_vect, IN harris_cfg_t *harris_param,
	OUT vect_desc_t *det_bit_vect, OUT vect_desc_t *resp_vect, OUT vect_desc_t *maxresp_vect);

/*!
 * This API performs Harris Point detection with an extended function
 * (supporting the NMS window size adaption). The data is stored on the
 * allocated memory with mfd.
 *
 * @param im_vect Input vector data of the image
 * @param harris_param Input harris response threshold, nms window size
 * @param det_bit_vect Output bit vector of Harris Points detection
 * @param resp_vect Output data vector of Harris response
 * @param maxresp_vect Output peak value of Harris response
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_harris_ext_mfd(IN vect_desc_mfd_t *im_vect, IN harris_cfg_t *harris_param,
	OUT vect_desc_mfd_t *det_bit_vect, OUT vect_desc_mfd_t *resp_vect, OUT vect_desc_mfd_t *maxresp_vect);

/*!
 * The initialization of the CVfilter function.
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cvfilter_init(void);

/*!
 * The release of the cvfilter function.
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cvfilter_release(void);

/*!
 * This API provides a way to do the filtering operation on an input vector,
 * which supports multiple channels filtering.
 *
 * @param im_vect Input data vector whose data format could be: 1. ufix8_0; 2. ufix8_8
 * @param ker_content Filter kernel content
 * @param ker_h Kernel Height
 * @param ker_w Kernel Width
 * @param out_vect Output vector whose data format could be:
 * 1. sfix16_0 (corresponds to input data format 1)
 * 2. sfix16_8 (corresponds to input data format 2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cvfilter(IN vect_desc_t *im_vect, IN float* ker_content,
	IN uint32_t ker_h, IN uint32_t ker_w, OUT vect_desc_t *out_vect);

/*!
 * This API provides a way to do the filtering operation on an input vector,
 * which supports multiple channels filtering. The data is stored on the
 * allocated memory with mfd.
 *
 * @param im_vect Input data vector whose data format could be: 1. ufix8_0; 2. ufix8_8
 * @param ker_content Filter kernel content
 * @param ker_h Kernel Height
 * @param ker_w Kernel Width
 * @param out_vect Output vector whose data format could be:
 * 1. sfix16_0 (corresponds to input data format 1)
 * 2. sfix16_8 (corresponds to input data format 2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cvfilter_mfd(IN vect_desc_mfd_t *im_vect, IN float* ker_content, IN uint32_t ker_h,
	IN uint32_t ker_w, OUT vect_desc_mfd_t *out_vect);

/*!
 * This API provides a way to do the filtering operation on an input vector,
 * which supports multiple channels filtering.
 *
 * @param im_vect Input data vector whose data format could be: 1. ufix8_0; 2. ufix8_8
 * @param ker_cfg Filter kernel configuration
 * @param out_vect Output vector whose data format could be:
 * 1. sfix16_0 (corresponds to input data format 1)
 * 2. sfix16_8 (corresponds to input data format 2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cvfilter_ext(IN vect_desc_t *im_vect, IN cvker_t* ker_cfg, OUT vect_desc_t *out_vect);

/*!
 * This API provides a way to do the filtering operation on an input vector in mfd
 * which supports multiple channels filtering.
 *
 * @param im_vect Input data vector whose data format could be: 1. ufix8_0; 2. ufix8_8
 * @param ker_cfg Filter kernel configuration
 * @param out_vect Output vector whose data format could be:
 * 1. sfix16_0 (corresponds to input data format 1)
 * 2. sfix16_8 (corresponds to input data format 2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cvfilter_mfd_ext(IN vect_desc_mfd_t *im_vect, IN cvker_mfd_t* ker_cfg, OUT vect_desc_mfd_t *out_vect);

/*!
 * This API provides a way to do the low pass filtering operation on an input
 * vector, which supports multiple channels filtering.
 *
 * @param im_vect Input data vector whose data format could be: 1. ufix8_0; 2. ufix8_8
 * @param ker_content Filter kernel content
 * @param ker_h Kernel Height
 * @param ker_w Kernel Width
 * @param out_vect Output vector whose data format could be:
 * 1. sfix16_0 (corresponds to input data format 1)
 * 2. sfix16_8 (corresponds to input data format 2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_lpcvfilter(IN vect_desc_t *im_vect, IN float* ker_content,
	IN uint32_t ker_h, IN uint32_t ker_w, OUT vect_desc_t *out_vect);

/*!
 * This API provides a way to do the low pass filtering operation on an input
 * vector, which supports multiple channels filtering. The data is stored on the
 * allocated memory with mfd.
 *
 * @param im_vect Input data vector whose data format could be: 1. ufix8_0; 2. ufix8_8
 * @param ker_content Filter kernel content
 * @param ker_h Kernel Height
 * @param ker_w Kernel Width
 * @param out_vect Output vector whose data format could be:
 * 1. sfix16_0 (corresponds to input data format 1)
 * 2. sfix16_8 (corresponds to input data format 2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_lpcvfilter_mfd(IN vect_desc_mfd_t *im_vect, IN float* ker_content, IN uint32_t ker_h,
	IN uint32_t ker_w, OUT vect_desc_mfd_t *out_vect);

/*!
 * This API provides a way to do the low pass filtering operation on an input
 * vector, which supports multiple channels filtering.
 *
 * @param im_vect Input data vector whose data format could be: 1. ufix8_0; 2. ufix8_8
 * @param ker_cfg Filter kernel configuration
 * @param out_vect Output vector whose data format could be:
 * 1. sfix16_0 (corresponds to input data format 1)
 * 2. sfix16_8 (corresponds to input data format 2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_lpcvfilter_ext(IN vect_desc_t *im_vect, IN cvker_t* ker_cfg, OUT vect_desc_t *out_vect);

/*!
 * This API provides a way to do the low pass filtering operation on an input in mfd
 * vector, which supports multiple channels filtering.
 *
 * @param im_vect Input data vector whose data format could be: 1. ufix8_0; 2. ufix8_8
 * @param ker_cfg Filter kernel configuration
 * @param out_vect Output vector whose data format could be:
 * 1. sfix16_0 (corresponds to input data format 1)
 * 2. sfix16_8 (corresponds to input data format 2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_lpcvfilter_mfd_ext(IN vect_desc_mfd_t *im_vect, IN cvker_mfd_t* ker_cfg, OUT vect_desc_mfd_t *out_vect);

/*!
 * The initialization of the morph function, which supports dilation and erosion.
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_morph_init(void);

/*!
 * The release of the morph function.
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_morph_release(void);

/*!
 * Dilate/Erode an image by using a specific structuring kernel.
 *
 * @param im_vect Input vector of binarized imag (0:black, 1:white)
 * @param out_vect Output vector of the binarized image (0:black, 1:white)
 * @param morph_h Kernel height
 * @param morph_w Kernel width
 * @param ker_data kernel data, e.g.[0,1,0;1,1,1;0,1,0]
 * @param morph_method Morph method selection: 0: Dilate 1: Erode
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_morph(IN vect_desc_t *im_vect, OUT  vect_desc_t *out_vect,
	IN int morph_h, IN int morph_w, IN float* ker_data, IN int morph_method);

/*!
 * Dilate/Erode an image by using a specific structuring kernel. The data is
 * stored on the allocated memory with mfd.
 *
 * @param im_vect Input vector of binarized imag (0:black, 1:white)
 * @param out_vect Output vector of the binarized image (0:black, 1:white)
 * @param morph_h Kernel height
 * @param morph_w Kernel width
 * @param ker_data kernel data, e.g.[0,1,0;1,1,1;0,1,0]
 * @param morph_method Morph method selection: 0: Dilate 1: Erode
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_morph_mfd(IN vect_desc_mfd_t *im_vect, OUT  vect_desc_mfd_t *out_vect,
	IN int morph_h, IN int morph_w, IN float* ker_data, IN int morph_method);

/*!
 * Dilate/Erode an image by using a specific structuring kernel.
 *
 * @param im_vect Input vector of binarized imag (0:black, 1:white)
 * @param out_vect Output vector of the binarized image (0:black, 1:white)
 * @param ker_cfg Kernel configuration
 * @param morph_method Morph method selection: 0: Dilate 1: Erode
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_morph_ext(IN vect_desc_t *im_vect,  OUT  vect_desc_t *out_vect, IN cvker_t* ker_cfg, IN int morph_method);

/*!
 * Dilate/Erode an image by using a specific structuring kernel in mfd.
 *
 * @param im_vect Input vector of binarized imag (0:black, 1:white)
 * @param out_vect Output vector of the binarized image (0:black, 1:white)
 * @param ker_cfg Kernel configuration
 * @param morph_method Morph method selection: 0: Dilate 1: Erode
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_morph_mfd_ext(IN vect_desc_mfd_t *im_vect, OUT vect_desc_mfd_t *out_vect, IN cvker_mfd_t* ker_cfg, IN int morph_method);


/*!
 * This API starts the perspective function, returning a non-null handler
 * after it loads.
 *
 * @param im_h The height of the input image
 * @param im_w The width of the input image
 * @return 0 = success, -1 = error.
 */
AMBA_API void* vproc_perspect_init(IN int im_h, IN int im_w);

/*!
 * Run this API to release the resources of perspective function.
 *
 * @param handler The processing handler of perspective
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_perspect_release(IN void* handler);

/*!
 * This API loads and updates the perspective warp matrix.  The warp matrix
 * must be loaded after the perspective initialization is successful.
 *
 * @param handler Input processing handler
 * @param prj_mat_vect Input perspective projection matrix
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_perspect_load_warp_mat(IN void* handler, IN vect_desc_t *prj_mat_vect);

/*!
 * This API loads and updates the perspective warp matrix.  The warp matrix
 * must be loaded after the perspective initialization is successful. The data
 * is stored on the allocated memory with mfd.
 *
 * @param handler Input processing handler
 * @param prj_mat_vect Input perspective projection matrix
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_perspect_load_warp_mat_mfd(IN void* handler, IN vect_desc_mfd_t *prj_mat_vect);

/*!
 * This API warps the image. Call this API after loading the warp matrix.
 * Note: The current version of the perspective function only supports the
 * input sizes 1280x800, 1280x720, and 720x640.
 *
 * @param handler Input processing handler
 * @param im_src Input vector of the image data
 * @param warp_out Output vector of the warp result
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_perspect_warp(IN void* handler, IN vect_desc_t *im_src, OUT vect_desc_t *warp_out);

/*!
 * This API warps the image. Call this API after loading the warp matrix.
 * Note: The current version of the perspective function only supports the
 * input sizes 1280x800, 1280x720, and 720x640. The data is stored on the
 * allocated memory with mfd.
 *
 * @param handler Input processing handler
 * @param im_src Input vector of the image data
 * @param warp_out Output vector of the warp result
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_perspect_warp_mfd(IN void* handler, IN vect_desc_mfd_t *im_src, OUT vect_desc_mfd_t *warp_out);

/*!
 * The initialization of the optical flow LK function.
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_optlk_init(void);

/*!
 * The release of the optical flow LK function.
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_optlk_release(void);

/*!
 * This API provides a way to do the basic LK calculation.
 *
 * @param im0_vect Input vector of image 0
 * @param im1_vect Input vector of image 1
 * @param optwin_data The weights of the LK windows. Default for 1.0
 * @param optwin_h LK windows height
 * @param optwin_w LK windows width
 * @param opt_param LK running parameters
 * @param dxy_vect Output vector that keeps the shift in x/y directions
 * @param peak_vect The peak response of the LK (same to harris response, R=det(M)-k*(trace(M))^2)
 * @return 0 = success, -1 = error.
 *
 * The following shows the calculation of LK optical flow
 * @image html optlk_calculation.png "calculation of LK optical flow"
 */
AMBA_API int vproc_optlk(IN vect_desc_t *im0_vect,  IN vect_desc_t *im1_vect,
	IN float* optwin_data, IN uint32_t optwin_h, IN uint32_t optwin_w, IN optlk_cfg_t *opt_param,
	OUT vect_desc_t *dxy_vect, OUT vect_desc_t *peak_vect);

/*!
 * This API provides a way to do the basic LK calculation. The data is stored
 * on the allocated memory with mfd.
 *
 * @param im0_vect Input vector of image 0
 * @param im1_vect Input vector of image 1
 * @param optwin_data The weights of the LK windows. Default for 1.0
 * @param optwin_h LK windows height
 * @param optwin_w LK windows width
 * @param opt_param LK running parameters
 * @param dxy_vect Output vector that keeps the shift in x/y directions
 * @param peak_vect The peak response of the LK (same to harris response, R=det(M)-k*(trace(M))^2)
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_optlk_mfd(IN vect_desc_mfd_t *im0_vect, IN  vect_desc_mfd_t *im1_vect,
	IN float* optwin_data, IN uint32_t optwin_h, IN uint32_t optwin_w, IN optlk_cfg_t *opt_param,
	OUT vect_desc_mfd_t *dxy_vect, OUT vect_desc_mfd_t *peak_vect);

/*!
 * This API provides a way to do the basic LK calculation.
 *
 * @param im0_vect Input vector of image 0
 * @param im1_vect Input vector of image 1
 * @param win_cfg The configuration of the LK windows.
 * @param opt_param LK running parameters
 * @param dxy_vect Output vector that keeps the shift in x/y directions
 * @param peak_vect The peak response of the LK (same to harris response, R=det(M)-k*(trace(M))^2)
 * @return 0 = success, -1 = error.
 *
 * The following shows the calculation of LK optical flow
 * @image html optlk_calculation.png "calculation of LK optical flow"
 */
AMBA_API int vproc_optlk_ext(IN vect_desc_t *im0_vect,  IN vect_desc_t *im1_vect,
	IN cvker_t* win_cfg, IN optlk_cfg_t *opt_param,
	OUT vect_desc_t *dxy_vect, OUT vect_desc_t *peak_vect);

/*!
 * This API provides a way to do the basic LK calculation in mfd.
 *
 * @param im0_vect Input vector of image 0
 * @param im1_vect Input vector of image 1
 * @param win_cfg The configuration of the LK windows.
 * @param opt_param LK running parameters
 * @param dxy_vect Output vector that keeps the shift in x/y directions
 * @param peak_vect The peak response of the LK (same to harris response, R=det(M)-k*(trace(M))^2)
 * @return 0 = success, -1 = error.
 *
 * The following shows the calculation of LK optical flow
 * @image html optlk_calculation.png "calculation of LK optical flow"
 */
AMBA_API int vproc_optlk_mfd_ext(IN vect_desc_mfd_t *im0_vect,  IN vect_desc_mfd_t *im1_vect,
	IN cvker_mfd_t* win_cfg, IN optlk_cfg_t *opt_param,
	OUT vect_desc_mfd_t *dxy_vect, OUT vect_desc_mfd_t *peak_vect);

/*!
 * This API performs the initialization of the cdist (cosine distance) function.
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cdist_init(void);

/*!
 * Call this API to release the resources of the cdist function.
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cdist_release(void);

/*!
 * The core function that performs the calculation of cosine distance for
 * two input vectors.
 *
 * @param in1_vect Input vector A
 * @param in2_vect Input vector B
 * @param cdist_val Output cosine distance value
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cdist(IN vect_desc_t *in1_vect,  IN vect_desc_t *in2_vect, OUT float *cdist_val);

/*!
 * The core function that performs the calculation of cosine distance for
 * two input vectors. The data is stored on the allocated memory with mfd.
 *
 * @param in1_vect Input vector A
 * @param in2_vect Input vector B
 * @param cdist_val Output cosine distance value
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cdist_mfd(IN vect_desc_mfd_t *in1_vect,  IN vect_desc_mfd_t *in2_vect, IN float *cdist_val);

/*!
 * The core function that performs the histogram calculation for an input
 * vector of a grayscale image.
 *
 * @param grayim_vect Input grayscale data vector
 * @param histout_vect Output histogram vector
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_imhist(IN vect_desc_t *grayim_vect,  OUT vect_desc_t *histout_vect);

/*!
 * The core function that performs the histogram calculation for an input
 * vector of a grayscale image. The data is stored on the allocated memory with mfd.
 *
 * @param grayim_vect Input grayscale data vector
 * @param histout_vect Output histogram vector
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_imhist_mfd(IN vect_desc_mfd_t *grayim_vect,  OUT vect_desc_mfd_t *histout_vect);

/*!
 * This API applies a fixed level threshold to the input vector data, and runs
 * the binarization.
 *
 * @param im_vect Input 2D vector
 * @param bw_vect Output vector of the binarized image
 * @param param Image binarization configuration parameters
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_bw(IN vect_desc_t *im_vect, OUT vect_desc_t *bw_vect, IN bw_cfg_t* param);

/*!
 * This API applies a fixed level threshold to the input vector data, and runs
 * the binarization. The data is stored on the allocated memory with mfd.
 *
 * @param im_vect Input 2D vector
 * @param bw_vect Output vector of the binarized image
 * @param param Image binarization configuration parameters
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_bw_mfd(IN vect_desc_mfd_t *im_vect, OUT vect_desc_mfd_t *bw_vect, IN bw_cfg_t* param);

/*!
 * This API performs the function of edge preserved noise reduction by mfd
 *
 * @param in:_vect Input 2D vector
 * @param out:_vect Output vector
 * @param dt: denoise alpha, suggest for 10.0
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_epnr_mfd(vect_desc_mfd_t *in_vect, vect_desc_mfd_t *out_vect, uint8_t dt);
/*!
 * This API performs the function of edge preserved noise reduction
 *
 * @param in:_vect Input 2D vector
 * @param out:_vect Output vector
 * @param dt: denoise alpha, suggest for 10.0
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_epnr(vect_desc_t *in_vect, vect_desc_t *out_vect, uint8_t dt);

/*!
 * This API performs the function of component connectivity labeling
 *
 * @param in_vect: Input vector
 * @param cfg:cclb parameters
 * @param out_vect: output vector
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cclb(IN vect_desc_t *in_vect, IN cclb_cfg_t *cfg, OUT vect_desc_t *out_vect);

/*!
 * This API performs the function of component connectivity labeling by mfd
 *
 * @param in_vect: Input vector
 * @param cfg:cclb parameters
 * @param out_vect: output vector
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_cclb_mfd(IN vect_desc_mfd_t *in_vect, IN cclb_cfg_t *cfg, OUT vect_desc_mfd_t *out_vect);

/*!
 * This API performs the initialization function of bayer patten to RGB/BGR
 * Notes: dedicated for the bayer pattern as below
 * G R G R
 * B G B G
 * G R G R
 * B G B G
 *
 * @param cfg: input height
 * @return >0 = success, NULL = error.
 */
AMBA_API void* vproc_bayer2bgr_init(bayer2bgr_cfg_t *cfg);

/*!
 * This API performs the release function of bayer patten to RGB/BGR
 *
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_bayer2bgr_release(void* handle);
/*!
 * This API performs the function of bayer patten to BGR/RGB
 *
 * @param handle: Input handler
 * @param in_vect: Input vector
 * @param out_vect: output vector
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_bayer2bgr(IN void* handle, IN vect_desc_t *in_vect, OUT vect_desc_t *out_vect);

/*!
 * This API performs the function of bayer patten to BGR/RGB by mfd
 *
 * @param handle: Input handler
 * @param in_vect: Input vector
 * @param out_vect: output vector
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_bayer2bgr_mfd(IN void* handle, IN vect_desc_mfd_t *in_vect, OUT vect_desc_mfd_t *out_vect);
/*!
 * This API copies data from source memory to destination memory with
 * a specified size.
 *
 * @param dst_phy_addr Destination data physical address
 * @param src_phy_addr Source data physical address
 * @param size Copy size
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_memcpy(IN uint32_t dst_phy_addr, IN uint32_t src_phy_addr, IN uint32_t size);

/*!
 * This API copies data from source memory to destination memory with
 * a specified size. The data is stored on the allocated memory with mfd.
 *
 * @param dst_mfd Destination data memory fd
 * @param dst_addr_offset Destination data memory offset
 * @param src_mfd Source data memory fd
 * @param src_addr_offset Source data memory offset
 * @param size Copy size
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_memcpy_mfd(IN int dst_mfd, IN uint32_t dst_addr_offset,
	IN int src_mfd, IN uint32_t src_addr_offset, IN uint32_t size);

/*!
 * This API set destination memory with a specified size by a given value.
 *
 * @param dst_phy_addr Destination data physical address
 * @param value A given value would be set
 * @param size Buffer size
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_memset(IN uint32_t dst_phy_addr, IN uint8_t value, IN uint32_t size);

/*!
 * This API set destination memory with a specified size by a given value.
 * The data is stored on the allocated memory with mfd.
 *
 * @param dst_mfd Destination data memory fd
 * @param dst_addr_offset Destination data memory offset
 * @param value A given value would be set
 * @param size Buffer size.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_memset_mfd(IN int dst_mfd, IN uint32_t dst_addr_offset, IN uint8_t value, IN uint32_t size);

/*!
 * Query the size of memory required for DSI splitting.
 *
 * @param dsi_w Width of DSI to be split
 * @param dsi_h Height of DSI to be split
 * @param size Required memory size
 * @return 0 = success, -1 = error.
 */
AMBA_API void dsi_split_query(IN uint32_t dsi_w, IN uint32_t dsi_h, OUT uint32_t *size);

/*!
 * Run DSI splitting on the input DSI.
 *
 * @param dsi_u16 Input DSI in UInt16 format
 * @param dsi_split_mem Physical address of allocated memory
 * @param dsi_split_virt Virtual address of allocated memory
 * @param msb_stream Output MSB stream
 * @param lsb_stream Output LSB stream
 * @return 0 = success, -1 = error.
 */
AMBA_API int dsi_split_run(IN vect_desc_t *dsi_u16, IN uint32_t dsi_split_mem,
	IN void *dsi_split_virt, OUT vect_desc_t *msb_stream, OUT vect_desc_t *lsb_stream);

/*!
 * Run DSI splitting on the input DSI. The data is stored on the allocated
 * memory with mfd.
 *
 * @param dsi_u16 Input DSI in UInt16 format
 * @param dsi_split_mfd Memory fd of allocated memory
 * @param dsi_split_moffset Memory offset of allocated memory
 * @param dsi_split_virt Virtual address of allocated memory
 * @param msb_stream Output MSB stream
 * @param lsb_stream Output LSB stream
 * @return 0 = success, -1 = error.
 */
AMBA_API int dsi_split_run_mfd(IN vect_desc_mfd_t *dsi_u16, IN int dsi_split_mfd,
	IN uint32_t dsi_split_moffset, IN void *dsi_split_virt,
	OUT vect_desc_mfd_t *msb_stream, OUT vect_desc_mfd_t *lsb_stream);

/*!
 * This API performs generic geometric transformation on image.
 *
 * @param src Input data vector in unsigned 8-bit format.
 * @param dst Output data vector in unsigned 8-bit format.
 * @param warp_fld Warp table in s11.4 format which is used to configure how the warp would be.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_warp(IN vect_desc_t *src, OUT vect_desc_t *dst,
	IN vect_desc_t *warp_fld);

/*!
 * This API performs generic geometric transformation on image. The data is
 * stored on the allocated memory with mfd.
 *
 * @param src Input data vector in unsigned 8-bit format.
 * @param dst Output data vector in unsigned 8-bit format.
 * @param warp_fld Warp table in s11.4 format which is used to configure how the warp would be.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_warp_mfd(IN vect_desc_mfd_t *src, OUT vect_desc_mfd_t *dst,
	IN vect_desc_mfd_t *warp_fld);

/*!
 * This API performs warp affine transformation on image.
 *
 * @param src Input vector.
 * @param dst Output vector.
 * @param waf_cfg Warp affine configuration.
 * @return 0 = success, -1 = error.
 *         -WARP_TABLE_OVERFLOW means if encounter a warp table conversion error.
 */
AMBA_API int vproc_warp_affine(IN vect_desc_t *src, OUT vect_desc_t *dst,
	IN warpaffine_cfg_t *waf_cfg);

/*!
 * This API performs warp affine transformation on image. The data is
 * stored on the allocated memory with mfd.
 *
 * @param src Input vector.
 * @param dst Output vector.
 * @param waf_cfg Warp affine configuration.
 * @return 0 = success, -1 = error.
 *         -WARP_TABLE_OVERFLOW means if encounter a warp table conversion error.
 */
AMBA_API int vproc_warp_affine_mfd(IN vect_desc_mfd_t *src, OUT vect_desc_mfd_t *dst,
	IN warpaffine_cfg_mfd_t *waf_cfg);

/*!
 * This API generates warp field for affine transformation.
 *
 * @param warp_fld Warp field that required for warpPerspective.
 * @param wpe_cfg Warp perspective configuration.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_gen_warppersp_field(OUT vect_desc_t *warp_fld,
	IN warppersp_cfg_t *wpe_cfg);

/*!
 * This API generates warp field for affine transformation. The data is
 * stored on the allocated memory with mfd.
 *
 * @param warp_fld Warp field that required for warpPerspective.
 * @param wpe_cfg Warp perspective configuration.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_gen_warppersp_field_mfd(OUT vect_desc_mfd_t *warp_fld,
	IN warppersp_cfg_mfd_t *wpe_cfg);

/*!
 * This API performs warp perspective transformation on image.
 *
 * @param src Input image.
 * @param dst Output image.
 * @param warp_fld Warp field converted from 3x3 conversion matrix.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_warp_persp(IN vect_desc_t *src, OUT vect_desc_t *dst,
	IN vect_desc_t *warp_fld);

/*!
 * This API performs warp perspective transformation on image. The data is
 * stored on the allocated memory with mfd.
 *
 * @param src Input image.
 * @param dst Output image.
 * @param warp_fld Warp field converted from 3x3 conversion matrix.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_warp_persp_mfd(IN vect_desc_mfd_t *src, OUT vect_desc_mfd_t *dst,
	IN vect_desc_mfd_t *warp_fld);

/*!
 * This API calculates statistics information of a vector, such as minimum,
 * maximum, mean and standard deviation.
 *
 * @param src Input vector.
 * @param dst Output that keeps desired statistics values in order of Min ,Max, Mean, and STD if all kept.
 * @param method_bmap statistics method packed in bitmap, which indicates standard deviation, mean, max and min from high to low.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_stats(IN vect_desc_t *src, OUT vect_desc_t *dst, IN uint32_t method_bmap);

/*!
 * This API calculates statistics information of a vector, such as minimum,
 * maximum, mean and standard deviation. The data is stored on the allocated
 * memory with mfd.
 *
 * @param src Input vector.
 * @param dst Output that keeps desired statistics values in order of Min ,Max, Mean, and STD if all kept.
 * @param method_bmap statistics method packed in bitmap, which indicates standard deviation, mean, max and min from high to low.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_stats_mfd(IN vect_desc_mfd_t *src, OUT vect_desc_mfd_t *dst, IN uint32_t method_bmap);

/*!
 * This API performs the DSI fusion on full FOV scale 0 and scale 2 disparity maps.
 *
 * @param scale0_dsi_packed10 Scale0’s disparity map in 10bitpacked format
 * @param scale2_dsi_packed10 Scale2’s disparity map in 10bitpacked format
 * @param fused_dsi Output fused disparity map in 8.4 format
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_dsi_fusion_2scales_ffov(IN vect_desc_t *scale0_dsi_packed10,
	IN vect_desc_t *scale2_dsi_packed10, OUT vect_desc_t *fused_dsi);

/*!
 * This API performs the DSI fusion on full FOV scale 0 and scale 2 disparity maps.
 * The data is stored on the allocated memory with mfd.
 *
 * @param scale0_dsi_packed10 Scale0’s disparity map in 10bitpacked format
 * @param scale2_dsi_packed10 Scale2’s disparity map in 10bitpacked format
 * @param fused_dsi Output fused disparity map in 8.4 format
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_dsi_fusion_2scales_ffov_mfd(IN vect_desc_mfd_t *scale0_dsi_packed10,
	IN vect_desc_mfd_t *scale2_dsi_packed10, OUT vect_desc_mfd_t *fused_dsi);

/*!
 * This API merges the U plane and V plane together to be an
 * element-interleaved UV.
 *
 * @param u U plane data.
 * @param v V plane data.
 * @param uv UV data.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_merge_uv(IN vect_desc_t *u, IN vect_desc_t *v,
	OUT vect_desc_t *uv);

/*!
 * This API merges the U plane and V plane together to be an
 * element-interleaved UV. The data is stored on the allocated memory with mfd.
 *
 * @param u U plane data.
 * @param v V plane data.
 * @param uv UV data.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_merge_uv_mfd(IN vect_desc_mfd_t *u, IN vect_desc_mfd_t *v,
	OUT vect_desc_mfd_t *uv);

/*!
 * This API split the U plane and V plane from an element-interleaved UV.
 *
 * @param uv UV data.
 * @param u U plane data.
 * @param v V plane data.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_split_uv(IN vect_desc_t *uv, IN vect_desc_t *u,
	OUT vect_desc_t *v);

/*!
 * This API split the U plane and V plane from an element-interleaved UV.
 * The data is stored on the allocated memory with mfd.
 *
 * @param uv UV data.
 * @param u U plane data.
 * @param v V plane data.
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_split_uv_mfd(IN vect_desc_mfd_t *uv, IN vect_desc_mfd_t *u,
	OUT vect_desc_mfd_t *v);

/*!
 * This API performs alpha blending for image A and image B by using an alpha
 * matrix.
 *
 * @param srcA input image A
 * @param srcB input image B
 * @param alpha input alpha matrix for blending
 * @param dst output blended image
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_alpha_blend(vect_desc_t *srcA, vect_desc_t *srcB,
	vect_desc_t *alpha, vect_desc_t *dst);

/*!
 * This API performs alpha blending for image A and image B by using an alpha
 * matrix. The data is stored on the allocated memory with mfd.
 *
 * @param srcA input image A
 * @param srcB input image B
 * @param alpha input alpha matrix for blending
 * @param dst output blended image
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_alpha_blend_mfd(vect_desc_mfd_t *srcA, vect_desc_mfd_t *srcB,
	vect_desc_mfd_t *alpha, vect_desc_mfd_t *dst);

/*!
 * This API converts the RGB image to YUV (NV12) format. The color space of
 * input is supposed to be plannar RGB or BGR.
 *
 * @param rgb RGB vector data descriptor
 * @param y Output Y channel vector data descriptor
 * @param uv Ouput UV channel vector data descriptor
 * @param rgb2yuv_mat Matrix data to convert RGB to YUV
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_rgb2yuv_420(vect_desc_t *rgb, vect_desc_t *y, vect_desc_t *uv,
	rgb2yuv_mat_t *rgb2yuv_mat);

/*!
 * This API converts the RGB image to YUV (NV12) format. The color space of
 * input is supposed to be plannar RGB or BGR. The data is stored on the
 * allocated memory with mfd.
 *
 * @param rgb RGB vector data descriptor
 * @param y Output Y channel vector data descriptor
 * @param uv Ouput UV channel vector data descriptor
 * @param rgb2yuv_mat Matrix data to convert RGB to YUV
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_rgb2yuv_420_mfd(vect_desc_mfd_t *rgb, vect_desc_mfd_t *y,
	vect_desc_mfd_t *uv, rgb2yuv_mat_t *rgb2yuv_mat);

/*!
 * This API transpose a vector. The output vector dimensional order is
 * configurable via the transpose_flag. Input and outout data format must be
 * the same.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param transpose_flag Flag to configure the output dimension
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_transpose(vect_desc_t *src,
	vect_desc_t *dst, uint32_t transpose_flag);

/*!
 * This API transpose a vector. The output vector dimensional order is
 * configurable via the transpose_flag. Input and outout data format must be
 * the same. The data is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @param transpose_flag Flag to configure the output dimension
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_transpose_mfd(vect_desc_mfd_t *src,
	vect_desc_mfd_t *dst, uint32_t transpose_flag);

/*!
 * This API computes the absolute value of each element of the input vector.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_abs(vect_desc_t *src, vect_desc_t *dst);

/*!
 * This API computes the absolute value of each element of the input vector.
 * The data is stored on the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_abs_mfd(vect_desc_mfd_t *src, vect_desc_mfd_t *dst);

/*!
 * This API flattens data to reduce DRAM comsuption.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_flatten(vect_desc_t *src, vect_desc_t *dst);

/*!
 * This API flattens data to reduce DRAM comsuption. The data is stored on
 * the allocated memory with mfd.
 *
 * @param src Input vector data descriptor
 * @param dst Output vector data descriptor
 * @return 0 = success, -1 = error.
 */
AMBA_API int vproc_flatten_mfd(vect_desc_mfd_t *src, vect_desc_mfd_t *dst);

/*! @} */ /* End of vproc-api-details */


#ifdef __cplusplus
}
#endif

#endif
