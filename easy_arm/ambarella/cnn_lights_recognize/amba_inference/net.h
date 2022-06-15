#ifndef __NET_H__
#define __NET_H__

#include <fcntl.h>
#include <nnctrl.h>
#include <cavalry_ioctl.h>
#include <cavalry_mem.h>
#include <vproc.h>

struct cv_mem {
	void *virt;
	unsigned long phys;
	unsigned long size;
};

typedef struct
{
    int                     netId;
    uint8_t                 cacheEn;
    struct net_cfg          stNetCfg;
    struct net_mem          stNetMem;
    struct net_input_cfg    stNetIn;
    struct net_output_cfg   stNetOut;
} NET_INFO_ST;

typedef struct
{
    void* addr1;
    void* addr2;
}BUFFER_ADDR_ST;

typedef struct
{
    int width;
    int height;
    int stride;
	BUFFER_ADDR_ST  addrVirt;
	BUFFER_ADDR_ST  addrPhys;
}IMAGE_INFO_ST;

#define NET_IN_MAX         (8)     /* 模型输入层最大个数 */
#define NET_OUT_MAX        (16)    /* 模型输出层最大个数 */
#define STRING_MAX              (256)
#define ALIGN64(x) ((x+63)&~(63))
#define MAX_SIZE 2048

/************************************************************************
* 函数名: AmbaGetVersion
* 功  能 ：获取当前版本号
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
int AmbaGetVersion();

int CavalryMemAlloc(OUT struct net_mem* mem, IN uint32_t psize, IN uint8_t cache_en);

/************************************************************************
* 函数名 ShowNetLayerInfo
* 功  能 ：打印模型信息
* 输  入 pstNNHandle -算法句柄地址
* 输  出 ：无
* 返回值 ：无
*************************************************************************/
void ShowNetLayerInfo(NET_INFO_ST *pstNet);

int PreProcess(IN const IMAGE_INFO_ST* const pstImage, OUT struct net_input_cfg* pstNetIn);

int Inference(NET_INFO_ST* pstNet);

#endif
