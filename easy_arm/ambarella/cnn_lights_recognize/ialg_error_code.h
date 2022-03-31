/***********************************************************************************
Copyright (c) 2018, Chongqing Unisinsight Technologies Co., Ltd. All rights reserved.
*
* @author:lixi<li.xi@unisinsight.com>
* @date: 2019/04/23
*
* @description: ������ͷ�ļ�
*
************************************************************************************/
#ifndef __IA_ERROR_CODE_H__
#define __IA_ERROR_CODE_H__

#define IALG_OK (0) /* �ɹ� */
#define IALG_NOK (-1) /* ͨ�ô��� */
#define IALG_BASE_ERR_ADDR (1000) /* ��������ʼ��ַ */
#define IALG_ERR_INVALID_PARAMS (IALG_BASE_ERR_ADDR + 1) /* �Ƿ����� */
#define IALG_ERR_FILE_NOT_EXIST (IALG_BASE_ERR_ADDR + 2) /* �ļ������� */
#define IALG_ERR_ENGINE_NOT_STARTUP (IALG_BASE_ERR_ADDR + 3) /* �㷨����δ���� */
#define IALG_ERR_GPU_NOT_EXIST (IALG_BASE_ERR_ADDR + 4) /* GPU�豸������ */
#define IALG_ERR_CHANNEL_NOT_EXIST (IALG_BASE_ERR_ADDR + 5) /* ��Ƶ��ͨ����Դ������ */
#define IALG_ERR_LICENSE (IALG_BASE_ERR_ADDR + 6) /* LICENSE���� */

#endif
