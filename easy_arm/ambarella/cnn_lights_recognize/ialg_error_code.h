/***********************************************************************************
Copyright (c) 2018, Chongqing Unisinsight Technologies Co., Ltd. All rights reserved.
*
* @author:lixi<li.xi@unisinsight.com>
* @date: 2019/04/23
*
* @description: 错误码头文件
*
************************************************************************************/
#ifndef __IA_ERROR_CODE_H__
#define __IA_ERROR_CODE_H__

#define IALG_OK (0) /* 成功 */
#define IALG_NOK (-1) /* 通用错误 */
#define IALG_BASE_ERR_ADDR (1000) /* 错误码起始地址 */
#define IALG_ERR_INVALID_PARAMS (IALG_BASE_ERR_ADDR + 1) /* 非法参数 */
#define IALG_ERR_FILE_NOT_EXIST (IALG_BASE_ERR_ADDR + 2) /* 文件不存在 */
#define IALG_ERR_ENGINE_NOT_STARTUP (IALG_BASE_ERR_ADDR + 3) /* 算法引擎未启动 */
#define IALG_ERR_GPU_NOT_EXIST (IALG_BASE_ERR_ADDR + 4) /* GPU设备不存在 */
#define IALG_ERR_CHANNEL_NOT_EXIST (IALG_BASE_ERR_ADDR + 5) /* 视频流通道资源不存在 */
#define IALG_ERR_LICENSE (IALG_BASE_ERR_ADDR + 6) /* LICENSE不足 */

#endif
