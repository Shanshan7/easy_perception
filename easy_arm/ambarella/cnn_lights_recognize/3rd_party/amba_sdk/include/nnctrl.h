/*************************************************************************************
 Copyright (c) 2018-2022, Unisinsight Technologies Co., Ltd. All rights reserved.
-------------------------------------------------------------------------------------
                            nnctrl.h
   Project Code: nnctrl数据类型定义
   Module Name :
   Date Created: 2021-2-4
   Author      :  
   Description : nnctrl数据类型定义
**************************************************************************************/
#ifndef _NNCTL_H_
#define _NNCTL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <cavalry_gen.h>
#include <cavalry_ioctl.h>

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/*! @addtogroup nnctrl-helper
 * @{
 */

/*!
 * @brief The version of nnctrl library
 */
struct nnctrl_version {
	uint32_t major;  /*!< Version major number */
	uint32_t minor;  /*!< Version minor number */
	uint32_t patch;  /*!< Version patch number */
	uint32_t cavalry_parser;  /*!< cavalry_parser version */
	uint32_t cavalry_compat;  /*!< cavalry_parser compat version */
	unsigned int mod_time;  /*!< Version modification time */
	char description[64];  /*!< Version description */
};

/*!
 *  @brief define the cavalry_gen major version
 */
#define GET_CAVALRY_MAJ_VER(v) (((v) & 0xff000000) >> 24)

/*!
 *  @brief define the cavalry_gen major version
 */
#define GET_CAVALRY_MIN_VER(v) (((v) & 0x00ff0000) >> 16)

/*!
 *  @brief define the cavalry_gen API version
 */
#define GET_CAVALRY_API_VER(v) (((v) & 0x0000ffff))

/*!
 *  @brief define the cavalry_gen API version that contain compat version info
 */
#define INCLUDE_COMPAT_API_VER (8)


/*!
 *  @brief define nnctrl library log level error
 */
#define NN_LOG_ERR     (0)

/*!
 *  @brief define nnctrl library log level warning
 */
#define NN_LOG_WARN    (1)

/*!
 *  @brief define nnctrl library log level info
 */
#define NN_LOG_INFO    (2)

/*!
 *  @brief define nnctrl library log level debug
 */
#define NN_LOG_DEBUG   (3)

/*!
 *  @brief define nnctrl library log level verbose
 */
#define NN_LOG_VERBOSE (4)

/*!
 *  @brief define nnctrl library log level default value
 */
#define NN_LOG_DEFAULT (NN_LOG_INFO)

/*!
 * @brief The network model binary version
 */
struct net_bin_version {
	IN const char *net_file;  /*!< The file name of NET.bin that was generated by cavalry_gen */

	OUT uint32_t cavalry_gen_version;  /*!< cavalry_gen version */
	OUT uint32_t cavalry_compat_version;  /*!< cavalry_compat version */

	OUT uint8_t vdg_arch;
	uint8_t reserved_0[3];

	uint32_t reserved[1];
};

/*!
 * @brief The network configuration in initialization
 */
struct net_cfg {
	IN const char *net_file;  /*!< The file name of NET.bin that was generated by cavalry_gen */
	IN const uint8_t *net_feed_virt;  /*!< Virtual memory of NET.bin.
		* Do not free this memory before call nnctrl_load_net() API */

	IN uint32_t verbose : 1;  /*!< The open verbose option of the network */
	IN uint32_t reuse_mem : 1;  /*!< The open reuse memory option to optimize the network size */
	IN uint32_t print_time : 1;  /*!< The open print vp_time and arm_time option */
	IN uint32_t no_dvi_mprot : 1;  /*!< No protection on dvi memory. Default is 0, turn on protection.
		* 0: turn on mprotect; 1: turn off mprotect */
	IN uint32_t no_chip_check : 1;  /*!< Don't check chip arch between NET.bin and libnnctrl.
		* 0: check chip arch; 1: don't check chip arch. It's debug proposal, shouldn't set this for production. */

	IN uint32_t net_all_input_no_mem : 1;  /*!< All network Input memory set no_mem */
	IN uint32_t net_all_output_no_mem : 1;  /*!< All network Output memory set no_mem */
	IN uint32_t enable_extra_high_mem : 1;  /*!< Load Net dvi and intermedia IO in high memory
		* which is outside of cpu access boundary */
	uint32_t reserved_0 : 8;  /*!< Reserved field */

	IN uint32_t net_circle_cnt : 16;  /*!< Duplicate dag_cnt and dag_desc N times in struct run_dags.
		* Networks's input and output should set no_mem flag.
		* Networks's input and output size are "port * net_circle_cnt" and the address are continuous.
		* It will save (N-1) ioctl interactive time between ARM and VP */

	IN uint32_t net_loop_cnt;  /*!< Network loop run internal count.
		* Blob memory size will be increased to "blob * net_loop_cnt".
		* The port size is "port * net_loop_cnt" and the address is continuous.
		* It will save (N-1) dvi loading time */

	OUT uint32_t net_mem_total;  /*!< The total network memory that is required.
		* The app should allocate this size for the network.  Every DVI and blob is aligned to 32-bit */
	OUT uint32_t dvi_mem_total;  /*!< The total memory of the DVI in the network
		* retrieved from the library */
	OUT uint32_t blob_mem_total;  /*!< The total memory allocated for the blob in the network.
		* It includes the input, the output, and the intermediate date.  It is obtained from the library */
	OUT uint32_t bandwidth_total;  /*!< The network access DRAM size during run */
	OUT uint32_t dvi_num_total;  /*!< The total DVI number of network */

	OUT uint32_t extra_high_mem_total;  /*!< Return extra high memory size if set enable_extra_high_mem */

	IN const struct customer_certificate *certificate_virt;  /*!< Should provide this for encrypted network.
		* Memory of Certificate. Do not free this memory before call nnctrl_load_net() API */
	IN const struct aes256_key *dek_encrypt_key_virt;  /*!< Should provide this for encrypted network.
		* Memory of DEK encrypted PEK Key. Do not free this memory before call nnctrl_load_net() API */
	IN const void *digest_virt;  /*!< Should provide this for encrypted network.
		* Memory of digest, size is 32 bytes, refer to @sa CAVALRY_DIGEST_LENGTH.
		* Do not free this memory before call nnctrl_load_net() API */

	uint32_t reserved_1[14];  /*!< Reserved field */
};

/*!
 * @brief The network memory description
 */
struct net_mem {
	uint8_t *virt_addr;  /*!< The virtual address mmap by app for network */
	uint32_t phy_addr;  /*!< The absolute physical address */
	uint32_t mem_size;  /*!< The memory size assigned by app */

	uint32_t extra_high_mem_phys;  /*!< The extra high memory absolute physcial address */

	uint32_t reserved[3];  /*!< Reserved field */
};

/*!
 * @brief The io dimension description
 */
struct io_dim {
	uint32_t plane;  /*!< The plane of the port */
	uint32_t depth;  /*!< The depth of the port */
	uint32_t height;  /*!< The height of the port */
	uint32_t width;  /*!< The width of the port */

	uint32_t pitch;  /*!< The pitch of the port in width alignment */
	uint32_t bitvector : 1;  /*!< If this port is a bit vector, one element is 1 bit in the DRAM if true */
	uint32_t is_variable : 1;  /*!< if it is scalar variable */
	uint32_t is_loop_pair : 1;  /*!< Indicate port is loop pair like LSTM layer. N = loop_cnt.
		* port_dram_size will be (N+1)*port_size, so there is one more port_size between batch port */
	uint32_t reserved_0 : 5;  /*!< Reserved field */

	uint32_t dram_fmt : 8;  /*!< The DRAM format of the port.  For internal use only */
	uint32_t loop_cnt : 16;  /*!< Indicator this port has loop cnt. N = loop_cnt.
		* port_dram_size will be N*port_size */

	uint32_t reserved_2[10];  /*!< Reserved field */
};

/*!
 * @brief The io data format description
 */
struct io_data_fmt {
	uint8_t sign;  /*!< 0: unsigned; 1: sign */
	uint8_t size;  /*!< 0: 8-bit;  1: 16-bit;  2: 32-bit;  3: 64-bit */
	int8_t expoffset;  /*!< Q value for quantized data */
	uint8_t expbits;  /*!< Exp bits for floating point */
};

/*!
 * @brief The bitmap of rotate and flip on port
 */
enum ROTATE_FILP_BITMAP {
	DROTATE_BIT = 0,
	HFLIP_BIT = 1,
	VFLIP_BIT = 2,
	DFLIP_BIT = 3,
	PFLIP_BIT = 4,
	ROTATE_FLIP_BIT_NUM,
};

/*!
 * @brief One input description
 */
struct input_desc {
	INOUT const char *name;  /*!< Input name that is set by the app,
		* or returned in nnctrl_get_net_io_cfg() API */
	INOUT uint8_t *virt;  /*!< Input virtual address. Return virtual address if not set no_mem */
	INOUT uint32_t addr;  /*!< Input absolute physical address.
		* Set by APP if set no_mem, otherwise get from Library */
	OUT uint32_t size;  /*!< Input data size obtained from the library */

	IN uint32_t no_mem : 1;  /*!< Let library allocate the memory for input if it equals 0.
		* The app allocates the memory and specifies the address if it equals 1 */

	IN uint32_t rotate_flip_bitmap : 5;  /*!< Bitmap for fives fields, ordered High -> Low.
				* 4: pflip; 3: dflip; 2: vflip; 1: hflip; 0: rotate. */
	uint32_t reserved_0 : 2;
	uint32_t reserved_1 : 8;  /*!< Reserved field */
	IN uint32_t update_pitch : 16;  /*!< Run-time changes the pitch of the input.
		* If rotation is used when the input height and width is different, it needs to be changed.
		* Do not subtract one on the real pitch, as the library will subtract one internally.
		* Can not less than width size, at least be 32 byte align. */

	OUT struct io_dim dim;  /*!< The dimension information for the port */
	OUT struct io_data_fmt data_fmt;  /*!< The data format of the port */

	uint32_t reserved_2[20];  /*!< Reserved field */
};

/*!
 * @brief One input description with memory fd
 */
struct input_mfd_desc {
	INOUT const char *name;  /*!< Input name that is set by the app,
		* or returned in nnctrl_get_net_io_cfg() API */
	INOUT uint8_t *virt;  /*!< Input virtual address. Return virtual address if not set no_mem */
	OUT unsigned long size;  /*!< Input data size obtained from the library */
	INOUT unsigned long fd_offset;  /*!< Input offset address base mem_fd.
		* Set by APP if set no_mem, otherwise get from Library */
	INOUT int mem_fd; /*!< Input address with memory fd.
		* Set by APP if set no_mem, otherwise get from Library */

	IN uint32_t no_mem : 1;  /*!< Let library allocate the memory for input if it equals 0.
		* The app allocates the memory and specifies the address if it equals 1 */

	IN uint32_t rotate_flip_bitmap : 5;  /*!< Bitmap for fives fields, ordered High -> Low.
				* 4: pflip; 3: dflip; 2: vflip; 1: hflip; 0: rotate. */

	uint32_t reserved_0 : 2;  /*!< Reserved field */
	uint32_t reserved_1 : 8;  /*!< Reserved field */
	IN uint32_t update_pitch : 16;  /*!< Run-time changes the pitch of the input.
		* If rotation is used when the input height and width is different, it needs to be changed.
		* Do not subtract one on the real pitch, as the library will subtract one internally.
		* Can not less than width size, at least be 32 byte align. */

	OUT struct io_dim dim;  /*!< The dimension information for the port */
	OUT struct io_data_fmt data_fmt;  /*!< The data format of the port */

	uint32_t reserved_2[20];  /*!< Reserved field */
};

/*!
 * @brief One output description
 */
struct output_desc {
	INOUT const char *name;  /*!< Output name that is set by the app,
		* or returned in nnctrl_get_net_io_cfg() API */
	INOUT uint8_t *virt;  /*!< Output virtual address. Return virtual address if not set no_mem */
	INOUT uint32_t addr;  /*!< Output absolute physical address.
		* Set by APP if set no_mem, otherwise get from Library */
	OUT uint32_t size;  /*!< Output data size obtained from the library */

	IN uint32_t no_mem : 1;  /*!< Let library allocate the memory for output if it equals 0.
		* The app allocates the memory and specifies the address if it equals 1 */
	IN uint32_t rotate_flip_bitmap : 5;  /*!< Bitmap for fives fields, ordered High -> Low.
				* 4: pflip; 3: dflip; 2: vflip; 1: hflip; 0: rotate. */
	uint32_t reserved_0 : 2;  /*!< Reserved field */
	uint32_t reserved_1 : 8;  /*!< Reserved field */
	IN uint32_t update_pitch : 16;	/*!< Run-time changes the pitch of the output.
		* Do not subtract one on the real pitch, as the library will subtract one internally.
		* Can not less than width size, at least be 32 byte align. */

	OUT struct io_dim dim;  /*!< The dimension information for the port */
	OUT struct io_data_fmt data_fmt;  /*!< The data format of the port */

	uint32_t reserved_2[20];  /*!< Reserved field */
};

/*!
 * @brief One output description with memory fd
 */
struct output_mfd_desc {
	INOUT const char *name;  /*!< Output name that is set by the app,
		* or returned in nnctrl_get_net_io_cfg() API */
	INOUT uint8_t *virt;  /*!< Output virtual address. Return virtual address if not set no_mem */
	OUT unsigned long size;  /*!< Output data size obtained from the library */
	INOUT unsigned long fd_offset;  /*!< Output offset address base mem_fd.
		* Set by APP if set no_mem, otherwise get from Library */
	INOUT int mem_fd; /*!< Output address with memory fd.
		* Set by APP if set no_mem, otherwise get from Library */

	IN uint32_t no_mem : 1;  /*!< Let library allocate the memory for output if it equals 0.
		* The app allocates the memory and specifies the address if it equals 1 */
	IN uint32_t rotate_flip_bitmap : 5;  /*!< Bitmap for fives fields, ordered High -> Low.
				* 4: pflip; 3: dflip; 2: vflip; 1: hflip; 0: rotate. */
	uint32_t reserved_0 : 2;  /*!< Reserved field */
	uint32_t reserved_1 : 8;  /*!< Reserved field */
	IN uint32_t update_pitch : 16;	/*!< Run-time changes the pitch of the output.
		* Do not subtract one on the real pitch, as the library will subtract one internally.
		* Can not less than width size, at least be 32 byte align. */

	OUT struct io_dim dim;  /*!< The dimension information for the port */
	OUT struct io_data_fmt data_fmt;  /*!< The data format of the port */

	uint32_t reserved_2[20];  /*!< Reserved field */
};

/*!
 *  @brief define max number of network's port (input and output)
 */
#define MAX_IO_NUM  (128)

/*!
 * @brief Total input in network
 */
struct net_input_cfg {
	INOUT uint32_t in_num;  /*!< Network Intput number, get from library when call
		* nnctrl_get_net_io_cfg() , otherwise specified by APP */
	struct input_desc in_desc[MAX_IO_NUM];  /*!< Network Intput descriptor */

	uint32_t reserved[4];  /*!< Reserved field */
};

/*!
 * @brief Total input in network with memory fd
 */
struct net_input_mfd_cfg {
	INOUT uint32_t in_num;  /*!< Network Intput number, get from library when call
		* nnctrl_get_net_io_cfg() , otherwise specified by APP */
	struct input_mfd_desc in_desc[MAX_IO_NUM];  /*!< Network Intput descriptor */

	uint32_t reserved[4];  /*!< Reserved field */
};

/*!
 * @brief Total output in network
 */
struct net_output_cfg {
	INOUT uint32_t out_num;  /*!< Network output number, get from library when call
		* nnctrl_get_net_io_cfg() , otherwise specified by APP */
	struct output_desc out_desc[MAX_IO_NUM];  /*!< Network Output descriptor */

	uint32_t reserved[4];  /*!< Reserved field */
};

/*!
 * @brief Total output in network with memory fd
 */
struct net_output_mfd_cfg {
	INOUT uint32_t out_num;  /*!< Network output number, get from library when call
		* nnctrl_get_net_io_cfg() , otherwise specified by APP */
	struct output_mfd_desc out_desc[MAX_IO_NUM];  /*!< Network Output descriptor */

	uint32_t reserved[4];  /*!< Reserved field */
};

/*!
 * @brief One of dvi configuration in network
 */
struct net_dvi_cfg {
	uint8_t *virt;  /*!< The absolute virtual address */
	uint32_t addr;  /*!< The absolute physical address */
	uint32_t size;  /*!< The dvi size */

	uint32_t reserved[28];  /*!< Reserved field */
};

/*!
 * @brief Run-time change network configuration
 */
struct net_run_cfg {
	uint32_t net_loop_cnt;  /*!< Changes the net_loop_cnt on-the-fly.
		* This value cannot be larger than the count specified while using nnctrl_init_net() */

	uint32_t single_dag_run : 1;  /*!< Runs the network DAG by DAG.
		* This for the debugging DAG execution time.  Disable it for a real case */
	uint32_t no_auto_resume : 1;  /*!< 0: Auto resume current net if other high priority net preempt;
		* 1: Abort run current net if other high priority net preempt */
	uint32_t reserved_0 : 6;  /*!< Reserved field */

	uint32_t split_num_run : 8;  /*!< Issued split number times ioctl to driver for each network.
		* Used case: When there are two independent networks in two threads or process,
		* one is large, another is small. Split large network to N part when issued ioctl,
		* then small network can get a chance to run instead of to waiting for large network finished */

	uint32_t reserved_1 : 16;  /*!< Reserved field */
	uint8_t priority;  /*!< The priority of each network, details refer to @cavalry_priority_t */
	uint8_t reserved_2[3];  /*!< Reserved field */
	uint32_t reserved_3[13];  /*!< Reserved field */
};

/*!
 * @brief The result information after network done
 */
struct net_result {
	float vp_time_us;  /*!< The time that the network spends in the Vector Processor (VP).
		* It contain blocking time by other network and real executing time */

	uint32_t reserved[15];  /*!< Reserved field */
};

/*! @macros AMBA_API
 *  @brief API function attribute */
#ifndef AMBA_API
#define AMBA_API __attribute__((visibility("default")))
#endif
/*! @} */ /* End of nnctrl-helper */


/*!
 * @addtogroup nnctrl-api-details
 * @{
 */

/*!
 * This API set log level of library, default is NN_LOG_DEFAULT.
 *
 * @param log_level the log level of nnctrl library.
 */
AMBA_API void nnctrl_set_log_level(IN uint32_t log_level);

/*!
 * This API initializes the library with the cavalry driver and library verbose configuration.
 * It must be called before all other functions.
 *
 * @param fd_cav the fd to open cavalry driver
 * @param verbose the flag to show verbose info in library
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_init(IN int fd_cav, IN uint8_t verbose);

/*!
 * This API get the version of library.
 *
 * @param ver the pointer to version
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_get_version(OUT struct nnctrl_version *ver);

/*!
 * This API get the cavalry_gen version of net binary.
 *
 * @param bin_ver the pointer to version
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_get_net_bin_version(struct net_bin_version *bin_ver);

/*!
 * This API suspends the current network from running, enabling the pending network
 * to enter the running state afterwards.  If it is set to quit_all, all running and pending networks
 * will be abandoned.  Users can reschedule the network after calling this function.
 *
 * @param quit_all the flag if quit all networks
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_suspend_net(IN uint8_t quit_all);

/*!
 * This API exit the library.
 * It must be called after all other functions.
 */
AMBA_API void nnctrl_exit(void);


/*!
 * This API initializes the Neural Network configuration.
 * Set network file name or network virtual address that contain NET.bin.
 * It will return the network ID and the network size that should been allocate by app later.
 *
 * @param net_cf the global configuration of the network
 * @param net_in the pointer to input of network. Can be NULL if no_mem is zero
 * @param net_out the pointer to output of network. Can be NULL if no_mem is zero
 * @return net_id >= 0, -1 = error.
 */
AMBA_API int nnctrl_init_net(INOUT struct net_cfg *net_cf,
	INOUT struct net_input_cfg *net_in, INOUT struct net_output_cfg *net_out);

/*!
 * This API initializes the Neural Network configuration with memory fd.
 * Set network file name or network virtual address that contain NET.bin.
 * It will return the network ID and the network size that should been allocate by app later.
 *
 * @param net_cf the global configuration of the network
 * @param net_in the pointer to input of network. Can be NULL if no_mem is zero
 * @param net_out the pointer to output of network. Can be NULL if no_mem is zero
 * @return net_id >= 0, -1 = error.
 */
AMBA_API int nnctrl_init_net_by_mfd(INOUT struct net_cfg *net_cf,
	INOUT struct net_input_mfd_cfg *net_in, INOUT struct net_output_mfd_cfg *net_out);

/*!
 * This API loads the network by the network ID with a specific memory address.
 * Then input and output address (virtual and physical) will be set.
 *
 * @param net_id the network ID
 * @param net_in the pointer to input of network. Can be NULL
 * @param net_out the pointer to output of network. Can be NULL
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_load_net(IN int net_id, IN struct net_mem *net_m,
	INOUT struct net_input_cfg *net_in, INOUT struct net_output_cfg *net_out);

/*!
 * This API loads the network by the network ID with a specific memory fd and offset.
 * Then input and output address (virtual and physical) will be set.
 *
 * @param net_id the network ID
 * @param net_in the pointer to input of network. Can be NULL
 * @param net_out the pointer to output of network. Can be NULL
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_load_net_by_mfd(IN int net_id, IN struct cavalry_mfd_desc *net_m,
	INOUT struct net_input_mfd_cfg *net_in, INOUT struct net_output_mfd_cfg *net_out);

/*!
 * This API gets the network input and output information, including the number, name, size etc.
 * It reports the physical and virtual address of the port if nnctrl_load_net is already called,
 * otherwise these are zero. At least, one of net_in and net_out should be valid.
 *
 * @param net_id the network ID
 * @param net_in the pointer to input of network. Can be NULL
 * @param net_out the pointer to output of network. Can be NULL
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_get_net_io_cfg(IN int net_id,
	OUT struct net_input_cfg *net_in, OUT struct net_output_cfg *net_out);

/*!
 * This API gets the network input and output information, including the number, name, size etc.
 * It reports the memory fd, offset and virtual address of the port if nnctrl_load_net is already called,
 * otherwise these are zero. At least, one of net_in and net_out should be valid.
 *
 * @param net_id the network ID
 * @param net_in the pointer to input of network. Can be NULL
 * @param net_out the pointer to output of network. Can be NULL
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_get_net_io_cfg_by_mfd(IN int net_id,
	OUT struct net_input_mfd_cfg *net_in, OUT struct net_output_mfd_cfg *net_out);

/*!
 * This API sets the network input and output physical address, rotate_flip_bitmap and
 * update_pitch.  Must set the IO no_mem flag in @ref nnctrl_init_net
 * if update the physical address. At least, one of net_in and net_out should be valid.
 *
 * @param net_id the network ID
 * @param net_in the pointer to input of network. Can be NULL
 * @param net_out the pointer to output of network. Can be NULL
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_set_net_io_cfg(IN int net_id,
	IN struct net_input_cfg *net_in, IN struct net_output_cfg *net_out);

/*!
 * This API sets the network input and output address with memory fd and offset,
 * rotate_flip_bitmap and update_pitch.  Must set the IO no_mem flag in @ref nnctrl_init_net
 * if update the io address. At least, one of net_in and net_out should be valid.
 *
 * @param net_id the network ID
 * @param net_in the pointer to input of network. Can be NULL
 * @param net_out the pointer to output of network. Can be NULL
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_set_net_io_cfg_by_mfd(IN int net_id,
	IN struct net_input_mfd_cfg *net_in, IN struct net_output_mfd_cfg *net_out);

/*!
 * This API allows the user to query the DVI description by the DVI ID.
 *
 * @param net_id the network ID
 * @param dvi_id the DVI ID
 * @param net_dvi the pointer to DVI configuration in network
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_query_dvi(IN int net_id, IN uint32_t dvi_id,
	OUT struct net_dvi_cfg *net_dvi);

/*!
 * This API run one specify DVI ID of network.
 *
 * @param net_id the network ID
 * @param dvi_id the DVI ID
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_run_one_dag_of_net(IN int net_id, IN uint32_t dvi_id,
	OUT struct net_result *net_ret);

/*!
 * This API run network with network ID after call @ref nnctrl_load_net
 *
 * @param net_id the network ID
 * @param net_ret the network running result
 * @param net_run the pointer to change the network cfg on-the-fly
 * @param net_in the pointer to input of network.  NULL: Use the previous input address.
 *        Valid Pointer: Network's input configuration. If the app allocates the input memory by itself,
 *        and the memory address is changed every time, the APP needs to specify this.
 *        Run-time update rotate-flip and udpate_pitch.
 * @param net_out the pointer to output of network. NULL: Use the previous input address.
 *        Valid Pointer: Network's output configuration. If the app allocates the output memory by itself,
 *        and this memory address is changed every time, the App needs to specify this.
 *        Run-time update rotate-flip and udpate_pitch.
 * @return 0 = success, -1 = error.
 *         -EINTR if it was interrupted by a signal; -EAGAIN if after call nnctrl_suspend_net()
 */
AMBA_API int nnctrl_run_net(IN int net_id,
	OUT struct net_result *net_ret, IN struct net_run_cfg *net_run,
	IN struct net_input_cfg *net_in, IN struct net_output_cfg *net_out);

/*!
 * This API run network with network ID with memory fd after call @ref nnctrl_load_net
 *
 * @param net_id the network ID
 * @param net_ret the network running result
 * @param net_run the pointer to change the network cfg on-the-fly
 * @param net_in the pointer to input of network.  NULL: Use the previous input address.
 *        Valid Pointer: Network's input configuration. If the app allocates the input memory by itself,
 *        and the memory address is changed every time, the APP needs to specify this.
 *        Run-time update rotate-flip and udpate_pitch.
 * @param net_out the pointer to output of network. NULL: Use the previous input address.
 *        Valid Pointer: Network's output configuration. If the app allocates the output memory by itself,
 *        and this memory address is changed every time, the App needs to specify this.
 *        Run-time update rotate-flip and udpate_pitch.
 * @return 0 = success, -1 = error.
 *         -EINTR if it was interrupted by a signal; -EAGAIN if after call nnctrl_suspend_net()
 */
AMBA_API int nnctrl_run_net_by_mfd(IN int net_id,
	OUT struct net_result *net_ret, IN struct net_run_cfg *net_run,
	IN struct net_input_mfd_cfg *net_in, IN struct net_output_mfd_cfg *net_out);

/*!
 * This API resumes the network with a network ID after nnctrl_run_net returns -EAGAIN.
 *
 * @param net_id the network ID
 * @param net_ret the network running result
 * @return 0 = success, -1 = error.
 *         -EINTR if it was interrupted by a signal; -EAGAIN if after call nnctrl_suspend_net()
 */
AMBA_API int nnctrl_resume_net(IN int net_id, OUT struct net_result *net_ret);

/*!
 * This API resumes the network with a network ID after nnctrl_run_net returns -EAGAIN.
 * The difference between @ref nnctrl_resume_net is this API support change net priority.
 *
 * @param net_id the network ID
 * @param net_run the pointer to change the network cfg on-the-fly. Support to change
 *        the priority when resume net.
 * @param net_ret the network running result
 * @return 0 = success, -1 = error.
 *         -EINTR if it was interrupted by a signal; -EAGAIN if after call nnctrl_suspend_net()
 */
AMBA_API int nnctrl_resume_net_ext(IN int net_id, IN struct net_run_cfg *net_run,
	OUT struct net_result *net_ret);

/*!
 * This API dumps the network DVI and blob data in a folder.
 *
 * @param net_id the network ID
 * @param path the path to save "dvi_dump" and "blob_dump" folder
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_dump_net(IN int net_id, IN const char *path);

/*!
 * This API get user defined custom info that inserted by user when gen network binary with cavalry_gen.
 *
 * @param net_id the network ID
 * @param the user info pointer to indicate the user data in memory.
 * @param the user info size.
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_get_net_user_info(IN int net_id,
	OUT void **user_virt, OUT uint32_t *size);

/*!
 * This API exits the network resource.
 *
 * @param net_id the network ID
 * @return 0 = success, -1 = error.
 */
AMBA_API int nnctrl_exit_net(IN int net_id);

/*! @} */ /* End of nnctrl-api-details */


#ifdef __cplusplus
}
#endif

#endif
