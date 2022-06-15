#include <unistd.h>
#include <cstdio>
#include <string.h>
#include "net.h"

/************************************************************************
* 函数名: AmbaGetVersion
* 功  能 ：获取当前版本号
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
int AmbaGetVersion()
{
    int                        s32Status = 0;
    struct cavalry_mem_version cavalryVer;
    struct nnctrl_version      nnctrlVer;
    struct vproc_version       vprocVer;

    s32Status = cavalry_mem_get_version(&cavalryVer);
    if (s32Status < 0)
    {
        printf("[%s][%d] get cavalry mem version err[%d]", __FUNCTION__, __LINE__, s32Status);
        return -1;
    }
    else
    {
        printf("%s: %u.%u.%u, mod-time: 0x%x",
               cavalryVer.description, cavalryVer.major, cavalryVer.minor, cavalryVer.patch, cavalryVer.mod_time);
    }

    s32Status = vproc_get_version(&vprocVer);
    if (s32Status < 0)
    {
        printf("[%s][%d] get vporc version err[%d]", __FUNCTION__, __LINE__, s32Status);
        return -1;
    }
    else
    {
        printf("%s: %u.%u.%u, mod-time: 0x%x",
               vprocVer.description, vprocVer.major, vprocVer.minor, vprocVer.patch, vprocVer.mod_time);
    }

    s32Status = nnctrl_get_version(&nnctrlVer);
    if (s32Status < 0)
    {
        printf("[%s][%d] get nnctrl get version err[%d]", __FUNCTION__, __LINE__, s32Status);
        return -1;
    }
    else
    {
        printf("%s: %u.%u.%u, mod-time: 0x%x",
               nnctrlVer.description, nnctrlVer.major, nnctrlVer.minor, nnctrlVer.patch, nnctrlVer.mod_time);
        printf("Cavalry Parser Version: %u.%u.%u",
               (nnctrlVer.cavalry_parser & 0xff000000) >> 24,
               (nnctrlVer.cavalry_parser & 0x00ff0000) >> 16,
               (nnctrlVer.cavalry_parser & 0x0000ffff));
    }

    return 0;
}

/************************************************************************
* 函数名: CavalryMemAlloc
* 功  能 ：申请vp内存
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
int CavalryMemAlloc(OUT struct net_mem* mem, IN uint32_t psize, IN uint8_t cache_en)
{
    unsigned long size  = psize;
    unsigned long phys  = 0;
    void*         virt  = NULL;

    if (cavalry_mem_alloc(&size, &phys, &virt, cache_en) < 0)
    {
        printf("[%s|%d] alloc_cv_mem failed!", __FUNCTION__, __LINE__);
        return -1;
    }
    mem->mem_size = size;
    mem->phy_addr = phys;
    mem->virt_addr = (uint8_t*)virt;

    return 0;
}

/************************************************************************
* 函数名 ShowNetLayerInfo
* 功  能 ：打印模型信息
* 输  入 ：pstNet -模型信息
* 输  出 ：无
* 返回值 ：无
*************************************************************************/
void ShowNetLayerInfo(NET_INFO_ST *pstNet)
{
    for (uint32_t i = 0; i < pstNet->stNetIn.in_num; ++i)
    {
        printf("Input: %u [%s] dim: (%u, %u, %u, %u), pitch: %u , dram_fmt: %u, bitvector: %d, data_fmt: (%d, %d, %d, %d), "
               "size: %u, phys: 0x%x, virt: %p\n",i, pstNet->stNetIn.in_desc[i].name,
               pstNet->stNetIn.in_desc[i].dim.plane,
               pstNet->stNetIn.in_desc[i].dim.depth,
               pstNet->stNetIn.in_desc[i].dim.height,
               pstNet->stNetIn.in_desc[i].dim.width,
               pstNet->stNetIn.in_desc[i].dim.pitch,
               pstNet->stNetIn.in_desc[i].dim.dram_fmt,
               pstNet->stNetIn.in_desc[i].dim.bitvector,
               pstNet->stNetIn.in_desc[i].data_fmt.sign,
               pstNet->stNetIn.in_desc[i].data_fmt.size,
               pstNet->stNetIn.in_desc[i].data_fmt.expoffset,
               pstNet->stNetIn.in_desc[i].data_fmt.expbits,
               pstNet->stNetIn.in_desc[i].size,
               pstNet->stNetIn.in_desc[i].addr,
               pstNet->stNetIn.in_desc[i].virt
        );
    }

    for (uint32_t i = 0; i < pstNet->stNetOut.out_num; ++i)
    {
        printf("Output: %u [%s] dim: (%u, %u, %u, %u),  pitch: %u , dram_fmt: %u, bitvector: %d, data_fmt: (%d, %d, %d, %d), "
               "size: %u, phys: 0x%x, virt: %p\n",i, pstNet->stNetOut.out_desc[i].name,
               pstNet->stNetOut.out_desc[i].dim.plane,
               pstNet->stNetOut.out_desc[i].dim.depth,
               pstNet->stNetOut.out_desc[i].dim.height,
               pstNet->stNetOut.out_desc[i].dim.width,
               pstNet->stNetOut.out_desc[i].dim.pitch,
               pstNet->stNetOut.out_desc[i].dim.dram_fmt,
               pstNet->stNetOut.out_desc[i].dim.bitvector,
               pstNet->stNetOut.out_desc[i].data_fmt.sign,
               pstNet->stNetOut.out_desc[i].data_fmt.size,
               pstNet->stNetOut.out_desc[i].data_fmt.expoffset,
               pstNet->stNetOut.out_desc[i].data_fmt.expbits,
               pstNet->stNetOut.out_desc[i].size,
               pstNet->stNetOut.out_desc[i].addr,
               pstNet->stNetOut.out_desc[i].virt
        );
    }
}

/************************************************************************
* 函数名 PreProcess
* 功  能 ：输入预处理
* 输  入 ：pstImage -图片信息
* 输  出 ：pstNetIn -模型输入层信息
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
int PreProcess(IN const IMAGE_INFO_ST* const pstImage, OUT struct net_input_cfg* pstNetIn)
{
    vect_desc_t yIn, yOut, uvIn, uvOut;
    memset(&yIn, 0, sizeof(yIn));
    memset(&yOut, 0, sizeof(yOut));
    memset(&uvIn, 0, sizeof(uvIn));
    memset(&uvOut, 0, sizeof(uvOut));

    yIn.shape.w     = pstImage->width;
    yIn.shape.h     = pstImage->height;
    yIn.shape.d     = 1;
    yIn.pitch       = ALIGN64(yIn.shape.w);
    yIn.color_space = CS_Y;
    yIn.data_addr   = (uint64_t)pstImage->addrPhys.addr1;
    yIn.roi.xoffset = 0;
    yIn.roi.yoffset = 0;
    yIn.roi.width   = pstImage->width;
    yIn.roi.height  = pstImage->height;


    uvIn.shape.w     = pstImage->width >> 1;
    uvIn.shape.h     = pstImage->height >> 1;
    uvIn.shape.d     = 2;
    uvIn.pitch       = yIn.pitch;
    uvIn.color_space = CS_ITL;
    uvIn.data_addr   = (uint64_t)pstImage->addrPhys.addr2;
    uvIn.roi.xoffset = 0;
    uvIn.roi.yoffset = 0;
    uvIn.roi.width   = (pstImage->width + 1) >> 1;
    uvIn.roi.height  = (pstImage->height + 1) >> 1;

    uint32_t u32NetIn_w  = pstNetIn->in_desc[0].dim.width;
    uint32_t u32NetIn_h  = pstNetIn->in_desc[0].dim.height;
    uint32_t u32NetIn_wh = pstNetIn->in_desc[0].dim.pitch * u32NetIn_h;
    memset(pstNetIn->in_desc[0].virt, 127, u32NetIn_wh);
    memset(pstNetIn->in_desc[1].virt, 128, u32NetIn_wh >> 1);
    
    /* 对输入图像数据刷新cache，以防外部输入的图像未cache同步造成数据不一致 */    
    cavalry_mem_sync_cache(pstNetIn->in_desc[0].size, pstNetIn->in_desc[0].addr, 1, 0);
    cavalry_mem_sync_cache(pstNetIn->in_desc[1].size, pstNetIn->in_desc[1].addr, 1, 0);

    yOut.shape.w      = u32NetIn_w;
    yOut.shape.h      = u32NetIn_h;
    yOut.shape.d      = 1;
    yOut.color_space  = CS_Y;
    yOut.pitch        = pstNetIn->in_desc[0].dim.pitch;
    yOut.data_addr    = pstNetIn->in_desc[0].addr;

    uvOut.shape.w     = u32NetIn_w >> 1;
    uvOut.shape.h     = u32NetIn_h >> 1;
    uvOut.shape.d     = 2;
    uvOut.color_space = CS_ITL;
    uvOut.pitch       = pstNetIn->in_desc[1].dim.pitch;
    uvOut.data_addr   = pstNetIn->in_desc[1].addr;

    /* y和uv通道分别缩放 */
    if (vproc_resize(&yIn, &yOut) < 0)
    {
        printf("[%s|%d] vproc_resize failure!", __FUNCTION__, __LINE__);
        return -1;
    }
    if (vproc_resize(&uvIn, &uvOut) < 0)
    {
        printf("[%s|%d] vproc_resize failure!", __FUNCTION__, __LINE__);
        return -1;
    }

    /* 刷新cache， 防止后面又会cpu对图像数据操作时数据不一致 */
    cavalry_mem_sync_cache(pstNetIn->in_desc[0].size, pstNetIn->in_desc[0].addr, 0, 1);
    cavalry_mem_sync_cache(pstNetIn->in_desc[1].size, pstNetIn->in_desc[1].addr, 0, 1);

    return 0;
}

/************************************************************************
* 函数名 Inference
* 功  能 ：模型推理
* 输  入 ：pstNet -模型信息
* 输  出 ：无
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
int Inference(NET_INFO_ST* pstNet)
{
    /* 此处必须刷新cache，memset在cpu中写入虚拟地址中，clean=1 invalide=0表示ARM写完，同步这一段数据到vp中 */
    for (uint32_t idx = 0; idx < pstNet->stNetOut.out_num; ++idx)
    {
        struct output_desc *pOutDesc = &pstNet->stNetOut.out_desc[idx];
        memset(pOutDesc->virt, 0, pOutDesc->size);
        cavalry_mem_sync_cache(pOutDesc->size, pOutDesc->addr, 1, 0);
    }
            
    struct net_result  net_ret = {0};
    struct net_run_cfg net_rev = {0};
    if (nnctrl_run_net(pstNet->netId, &net_ret, &net_rev, NULL, NULL) < 0)
    {
        printf("[%s|%d] nnctrl_run_net err.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    /* 此处必须刷新cache，vp处理之后数据在物理地址中，clean=0 invalide=1表示ARM需要读这一段内存 */
    for(uint32_t i = 0; i < pstNet->stNetOut.out_num; ++i)
    {
        cavalry_mem_sync_cache(pstNet->stNetOut.out_desc[i].size, pstNet->stNetOut.out_desc[i].addr, 0, 1);
    }

    return 0;
}
