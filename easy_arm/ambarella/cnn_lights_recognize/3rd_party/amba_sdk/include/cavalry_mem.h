/*************************************************************************************
 Copyright (c) 2018-2022, Unisinsight Technologies Co., Ltd. All rights reserved.
-------------------------------------------------------------------------------------
                            cavalry_mem.h
   Project Code: cavalry_mem数据类型定义
   Module Name :
   Date Created: 2021-2-4
   Author      :  
   Description : cavalry_mem数据类型定义
**************************************************************************************/
#ifndef _CAVALRY_MEM_H_
#define _CAVALRY_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/*! @addtogroup cavalry_mem-helper
 * @{
 */

/*!
 * @brief The version of cavalry_mem library
 */
struct cavalry_mem_version {
	uint32_t major;  /*!< Version major number */
	uint32_t minor;  /*!< Version minor number */
	uint32_t patch;  /*!< Version patch number */
	unsigned int mod_time;  /*!< Version modification time */
	char description[64];  /*!< Version description */
};

/*! @macros AMBA_API
 * @brief API function attribute */
#ifndef AMBA_API
#define AMBA_API __attribute__((visibility("default")))
#endif
/*! @} */ /* End of cavalry_mem-helper */


/*!
 * @addtogroup cavalry_mem-api-details
 * @{
 */

/*!
 * This API initializes the library with the cavalry driver and library verbose configuration.
 * It must be called before all other functions
 *
 * @param fd_cav the fd to open cavalry driver
 * @param verbose the flag to show verbose info in library
 * @return 0 = success, -1 = error.
 */
AMBA_API int cavalry_mem_init(IN int fd_cav, IN uint8_t verbose);

/*!
 * This API return the initialized cavalry driver fd when call @ref cavalry_mem_init().
 *
 * @return -1 = error, otherwise success.
 */
AMBA_API int cavalry_mem_get_fd(void);

/*!
 * This API gets the version of library.
 *
 * @param ver the pointer to version
 * @return 0 = success, -1 = error.
 */
AMBA_API int cavalry_mem_get_version(struct cavalry_mem_version *ver);

/*!
 * This API exits the library.
 * It must be called after all other functions
 */
AMBA_API void cavalry_mem_exit(void);

/*!
 * This API allocates the memory from the CV user memory.
 * If process have not free memory before exit, driver will auto recycle these leaked memory.
 * Cache memory exist between ARM and DRAM.
 * Turn on @param cache_en can boost ARM process speed.
 *
 * @param psize the pointer to total size want allocate
 * @param pphys the pointer to physical address that return
 * @param pvirt the pointer to virtual address that return
 * @param cache_en the flag to enable cached memory. 0: non-cache; 1: cache
 * @return 0 = success, -1 = error.  -EBUSY if memory is busy, should retry alloc it.
 */
AMBA_API int cavalry_mem_alloc(IN unsigned long *psize,
	OUT unsigned long *pphys, OUT void **pvirt, IN uint8_t cache_en);


/*!
 * This API allocates the memory from the CV user memory.
 * If process have not free memory before exit, driver DO NOT auto recycle these leaked memory.
 * Then it cause memory leakage issue. To avoid this issue, please use API cavalry_mem_alloc().
 * Cache memory exist between ARM and DRAM.
 * Turn on @param cache_en can boost ARM process speed.
 *
 * @param psize the pointer to total size want allocate
 * @param pphys the pointer to physical address that return
 * @param pvirt the pointer to virtual address that return
 * @param cache_en the flag to enable cached memory. 0: non-cache; 1: cache
 * @return 0 = success, -1 = error.  -EBUSY if memory is busy, should retry alloc it.
 */
AMBA_API int cavalry_mem_alloc_persist(IN unsigned long *psize,
	OUT unsigned long *pphys, OUT void **pvirt, IN uint8_t cache_en);


/*!
 * This API allocates the memory from the CV user memory by supply file description.
 * This API always auto recycle leaked memory.
 * Cache memory exist between ARM and DRAM.
 * Turn on @param cache_en can boost ARM process speed.
 *
 * @param psize the pointer to total size want allocate
 * @param fd the pointer of memory file description that return
 * @param pvirt the pointer to virtual address that return
 * @param cache_en the flag to enable cached memory. 0: non-cache; 1: cache
 * @return 0 = success, -1 = error.  -EBUSY if memory is busy, should retry alloc it.
 */
AMBA_API int cavalry_mem_alloc_mfd(IN unsigned long size,
	OUT int *fd, OUT void **pvirt, IN uint8_t cache_en);

/*!
 * This API frees the memory from the CV user memory.
 *
 * @param size total size
 * @param phys physical address
 * @param virt virtual address
 * @return 0 = success, -1 = error.
 */
AMBA_API int cavalry_mem_free(IN unsigned long size,
	IN unsigned long phys, IN void *virt);

/*!
 * This API frees the memory from the CV user memory by file description.
 *
 * @param size total size
 * @param fd the file description of memory
 * @param virt virtual address
 * @return 0 = success, -1 = error.
 */
AMBA_API int cavalry_mem_free_mfd(IN unsigned long size,
	IN int fd, IN void *virt);

/*!
* This API syncs the cached memory when cache_en is set in @ref cavalry_mem_alloc.
* It will return an error if it is used with non-cached memory.
* Can sync one slice of total memory. At the begin need sync network's dvi memory after nnctrl_load_net,
* then most of time sync Network's Input/Ouput memory.
*
* @param size size of memory
* @param phys physical address
* @param virt virtual address
* @param clean the flag to clean cache.  0: turn off; 1: turn on.
*                        Program Flow: 1.ARM write -> 2.clean cache -> 3.VP read
* @param invalid the flag to invalid cache.  0: turn off; 1: turn on.
*                        Program Flow: 1.VP write -> 2.invalid cache -> 3.ARM read
* @return 0 = success, -1 = error.
*/
AMBA_API int cavalry_mem_sync_cache(
	IN unsigned long size, IN unsigned long phys,
	IN uint8_t clean, IN uint8_t invalid);

/*!
* This API syncs the cached memory when cache_en is set in @ref cavalry_mem_alloc.
* It will return an error if it is used with non-cached memory.
* Can sync one slice of total memory. At the begin need sync network's dvi memory after nnctrl_load_net,
* then most of time sync Network's Input/Ouput memory.
*
* @param size size of memory
* @param offset the offset base on file description of memory
* @param fd the file description of memory
* @param virt virtual address
* @param clean the flag to clean cache.  0: turn off; 1: turn on.
*                        Program Flow: 1.ARM write -> 2.clean cache -> 3.VP read
* @param invalid the flag to invalid cache.  0: turn off; 1: turn on.
*                        Program Flow: 1.VP write -> 2.invalid cache -> 3.ARM read
* @return 0 = success, -1 = error.
*/
AMBA_API int cavalry_mem_sync_cache_mfd(
	IN unsigned long size, IN unsigned long offset, IN int fd,
	IN uint8_t clean, IN uint8_t invalid);

/*!
 * This API convert user space virt address to physical address.
 *
 * @param virt virtual address in user space
 * @return 0 = error, non-zero = success.
 */
AMBA_API unsigned long cavalry_mem_virt_to_phys(IN void *virt);

/*!
 * This API convert physical address to user space virt address.
 *
 * @param phys physical address
 * @return NULL = error, valid pointer = success.
 */
AMBA_API void *cavalry_mem_phys_to_virt(IN unsigned long phys);

/*!
 * This API get memory size by virt.
 *
 * @param virt virtual address in user space
 * @return 0 = error, non-zero = success.
 */
AMBA_API unsigned long cavalry_mem_get_size_by_virt(IN void *virt);

/*! @} */ /* End of cavalry_mem-api-details */

#ifdef __cplusplus
}
#endif

#endif
