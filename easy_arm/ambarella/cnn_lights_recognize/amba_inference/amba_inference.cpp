#include "amba_inference.h"

Amba_Inference::Amba_Inference()
{
    /* 模型信息，在编译之前要自定义修改 */
    int   netInNum  = 2;                                           //输入层个数
    char  netInName[NET_IN_MAX][STRING_MAX] = {"data", "data_uv"}; //输入层名
    int   netOutNum = 1;                                           //输出层个数
    char  netOutName[NET_OUT_MAX][STRING_MAX] = {"prob"};          //输出层名
    char  netFile[STRING_MAX] = "cavalry_googlenet_yuv.bin";       //模型路径

    /* vproc.bin路径，在编译之前要自定义修改 */
     char VPROC_BIN_PATH[STRING_MAX] = "/usr/local/vproc/vproc.bin";

    int             fdCavalry = -1; // cavalry设备句柄，不要改
    uint8_t         verbose  = 0;
    int             nnCnt     = 0;  // 算子计数器，退出安霸环境时需要
    int             fdFlag    = 0;  // cavalry设备句柄
    struct net_mem  stBinMem  = {0};// vproc.bin


    std::ifstream in(json_path, std::ios::binary);
    Json::Reader reader;
    Json::Value root;
//
    if(reader.parse(in, root))
    {
        amba_path=root["amba_path"].asString();
    }


    else
    {
        std::cout << "Error opening file\n";
        exit(0);
    }
}

Amba_Inference::~Amba_Inference()
{
    
}


/************************************************************************
* 函数名: AmbaEntry
* 功  能 ：初始化，只需要运行一次
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
int Amba_Inference::AmbaEntry()
{
    fdCavalry = cavalry_mem_get_fd();//获取已初始化的 CV 文件描述符
    if (-1 == fdCavalry)
    {
        /************ open cavalry dev ************/
        fdCavalry = open(CAVALRY_DEV_NODE, O_RDWR, 0);
        if (fdCavalry < 0)
        {
            printf("[%s|%d] open cavalry dev failed!\n", __FUNCTION__, __LINE__);
            return -1;
        }
        printf("open cavalry dev OK.\n");

        /************ cavalry memery init ************/
        if (cavalry_mem_init(fdCavalry, verbose) < 0)//初始化 CV 内存和设置详细日志配置,-1：失败
        {
            printf("[%s|%d] cavalry_mem_init failed!\n", __FUNCTION__, __LINE__);
            return -1;
        }
        printf("cavalry_mem_init OK.\n");

        fdFlag = 1;
    }

    return 0;
}

/************************************************************************
* 函数名: AmbaExit
* 功  能 ：反初始化，只需要运行一次
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
void Amba_Inference::AmbaExit(NET_INFO_ST* pstNet)
{
    /************ nnctrl deinit ************/
    struct net_mem* pstNetMem = &pstNet->stNetMem;//神经网络内存描述结构体
    if (NULL != pstNetMem->virt_addr)//虚拟地址不为空
    {
        cavalry_mem_free(pstNetMem->mem_size, pstNetMem->phy_addr, pstNetMem->virt_addr);//释放 CV 内存。
    }

    if (pstNet->netId >= 0)
    {
        nnctrl_exit_net(pstNet->netId);//销毁指定神经网络的资源。
        pstNet->netId = -1;
    }

    nnCnt--;
    if (-1 != fdCavalry && 0 == nnCnt)
    {
        if (NULL != stBinMem.virt_addr)
        {
            cavalry_mem_free(stBinMem.mem_size, stBinMem.phy_addr, stBinMem.virt_addr);
        }
        nnctrl_exit();//销毁神经网络库。

        /************ vproc deinit ************/
        vproc_exit();

        /************ cavalry deinit ************/
        if (1 == fdFlag)
        {
            if (fdCavalry >= 0)
            {
                close(fdCavalry);
                fdCavalry = -1;
            }
            cavalry_mem_exit();
        }
        fdCavalry = -1;
    }
}

/************************************************************************
* 函数名：Init
* 功  能 ：模型加载
* 输  入 ：pstNet -模型信息
* 输  出 ：无
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
int Amba_Inference::Init(NET_INFO_ST* pstNet)
{
    uint32_t vprocSize = 0;
    memset(pstNet, 0, sizeof(NET_INFO_ST));
    pstNet->netId = -1;
    pstNet->cacheEn = 1;

    /************ vproc init ************/
    if (vproc_init(VPROC_BIN_PATH, &vprocSize) < 0)
    {
        printf("[%s|%d] vproc_init failed!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if (CavalryMemAlloc(&stBinMem, vprocSize, pstNet->cacheEn) < 0)//分配 CV 内存
    {
        printf("[%s|%d] alloc_cv_mem failed!\n", __FUNCTION__, __LINE__);
        return -1;
    }
    vproc_load(fdCavalry, stBinMem.virt_addr, stBinMem.phy_addr, stBinMem.mem_size);

    /************ nnctrl init ************/
    if (nnctrl_init(fdCavalry, verbose) < 0)//加载神经网络库。
    {
        printf("[%s|%d] nnctrl_init failed!\n", __FUNCTION__, __LINE__);
        return -1;
    }
    printf("nnctrl_init OK.\n");

    nnCnt++;

    /************ net init ************/
    // u32NetOutNum 或 netInName 为0则不做 net init
    if(0 == netInNum || 0 == netOutNum)
    {
        printf("[%s|%d] without net input or output!\n", __FUNCTION__, __LINE__);
        return -1;
    }
    pstNet->stNetCfg.verbose     = 0;
    pstNet->stNetCfg.reuse_mem   = 1;
    pstNet->stNetCfg.print_time  = 0;
    pstNet->stNetCfg.net_file    = netFile;

    pstNet->stNetIn.in_num = netInNum;
    for (int i = 0; i < netInNum; ++i)
    {
        pstNet->stNetIn.in_desc[i].name = netInName[i];
    }

    pstNet->stNetOut.out_num = netOutNum;
    for (int i = 0; i < netOutNum; ++i)
    {
        pstNet->stNetOut.out_desc[i].name = netOutName[i];
    }

    pstNet->netId = nnctrl_init_net(&pstNet->stNetCfg, &pstNet->stNetIn, &pstNet->stNetOut);//初始化神经网络配置。
    if (pstNet->netId < 0)
    {
        printf("[%s|%d] nnctrl_init_net failed!", __FUNCTION__, __LINE__);
        return -1;
    }
    else if (0 == pstNet->stNetCfg.net_mem_total)
    {
        printf("[%s|%d] nnctrl_init_net get total size is zero!", __FUNCTION__, __LINE__);
        return -1;
    }
    printf("nnctrl_init_net ok.\n");

    /************ cavalry memery alloc ************/
    if (CavalryMemAlloc(&pstNet->stNetMem, pstNet->stNetCfg.net_mem_total, pstNet->cacheEn) < 0)
    {
        printf("[%s|%d] CavalryMemAlloc err, size:%d", __FUNCTION__, __LINE__, pstNet->stNetMem.mem_size);
        return -1;
    }

    /************ nnctrl load net ************/
    if (nnctrl_load_net(pstNet->netId, &pstNet->stNetMem, &pstNet->stNetIn, &pstNet->stNetOut) < 0)//加载神经网络到指定的内存地址并设置到输入输出地址。
    {
        printf("[%s|%d] nnctrl load net failed!", __FUNCTION__, __LINE__);
        return -1;
    }
    if (pstNet->cacheEn)
    {
        cavalry_mem_sync_cache(pstNet->stNetMem.mem_size, pstNet->stNetMem.phy_addr, 1, 0);
    }

    /* 打印模型输入输出信息 */
    if(NULL != pstNet->stNetCfg.net_file)
    {
        printf("load model:%s\n", pstNet->stNetCfg.net_file);
        ShowNetLayerInfo(pstNet);
    }
    
    return 0;
}

/************************************************************************
* 函数名 PostProcess
* 功  能 ：输出后处理
* 输  入 ：pstNet -模型信息
* 输  出 ：output -模型输出
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
void Amba_Inference::PostProcess(NET_INFO_ST* pstNet, std::vector<float>& output)
{
    float thresh = 0.f;
    int type = 0;
    float conf = 0.f;
    for (uint32_t i = 0; i < pstNet->stNetOut.out_num; ++i)
    {
        struct io_dim layerDim = pstNet->stNetOut.out_desc[i].dim;//IO 维度描述结构体
        float*        prob     = (float*)pstNet->stNetOut.out_desc[i].virt;
        int outSize = layerDim.depth * layerDim.height * layerDim.width;
        for(int j=0; j < outSize; j++)
        {
            output.push_back(prob[j]);
            if (prob[j] > thresh)
            {
                thresh = prob[j];
                conf = prob[j] * 100;
                type = j;
            }
        }
    }
    printf("layerOutput : type = %d, conf = %f\n", type, conf);
}

/************************************************************************
* 函数名 LoadImgFile
* 功  能 ：加载图片，将opencv读入的Mat个数图片，转换成nv12的格式
* 输  入 ：img -图片
* 输  出 ：pstImage -图像数据
* 返回值 ：无
*************************************************************************/
void Amba_Inference::LoadImgFile(IN cv::Mat& img, OUT IMAGE_INFO_ST* pstImage)
{
    cv::Mat resizeImg, cvtImg;
    int w = img.cols % 2 == 0 ? img.cols : (img.cols - 1);
    int h = img.rows % 2 == 0 ? img.rows : (img.rows - 1);
    int stride = ALIGN64(w);
    cv::resize(img, resizeImg, cv::Size(w, h));
    cv::cvtColor(resizeImg, cvtImg, cv::COLOR_BGR2YUV_I420);
    memcpy((uchar*)pstImage->addrVirt.addr1, cvtImg.data, w*h*3/2);

    /* 将YUV_I420格式数据转成NV12格式 */
    uchar* u_tmp = cvtImg.data + w*h;
    uchar* v_tmp = cvtImg.data + w*h*5/4;
    uchar* uv = (uchar*)pstImage->addrVirt.addr1 + w*h;
    int y_size = w*h;
    int jj = 0, ii = 0;
    for( ; jj < y_size/2; jj+=2, ii++)
    {
        uv[jj] = u_tmp[ii];
        uv[jj+1] = v_tmp[ii];
    }

    /* 在cv2x设备上数据必须保证64字节对齐，如果图像宽度不能被64整除，必须按64字节对齐的宽度补边 */
    uchar *pTmp = (uchar*)malloc(stride*h*3/2);
    uchar *pSrc = (uchar*)pstImage->addrVirt.addr1;
    uchar *pDst = pTmp;
    for(int i = 0; i < h*3/2; ++i)
    {
        memcpy(pDst, pSrc, w);
        pDst += stride;
        pSrc += w;
    }
    memcpy(pstImage->addrVirt.addr1, pTmp, stride*h*3/2);
    free(pTmp);
    pTmp = NULL;

    pstImage->addrVirt.addr2 = (void*)((uchar*)pstImage->addrVirt.addr1 + stride*h);
    pstImage->addrPhys.addr2 = (void*)((uchar*)pstImage->addrPhys.addr1 + stride*h);
    pstImage->width      = w;
    pstImage->height     = h;
    pstImage->stride     = stride;

    /* 此处必须刷新cache，数据目前在cpu虚拟地址中，clean=1 invalide=0表示ARM写完，同步这一段数据到vp中 */
    cavalry_mem_sync_cache(stride*h*3/2, (long unsigned int)pstImage->addrPhys.addr1, 1, 0);//同步 cached 的 CV 内存
}

/************************************************************************
* 函数名 Net
* 功  能 ：模型加载+运行
* 输  入 ：img -opencv读入的图片信息
* 输  出 ：output -模型输出
* 返回值 ：成功：0
*         失败：错误码
*************************************************************************/
int Amba_Inference::Net(IN cv::Mat& img, OUT std::vector<float>& output)
{
    unsigned long size = MAX_SIZE * MAX_SIZE *3;
    void* virt = NULL;
    unsigned long phys = 0;
    IMAGE_INFO_ST stImage = {0};
    NET_INFO_ST stNet = {0};

    /* 资源未初始化时只初始化一次 */
    if(AmbaEntry() < 0)
    {
        printf("[%s|%d] AmbaEntry failed!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    do {
        if(Init(&stNet) != 0) {
            printf("[%s|%d] Init failed!\n", __FUNCTION__, __LINE__);
            break;
        }

        if(cavalry_mem_alloc(&size, &phys, &virt, 1) < 0) {
            printf("[%s|%d] cavalry_mem_alloc failed!", __FUNCTION__, __LINE__);
            break;
        }
        stImage.addrPhys.addr1 = (void *)phys;
        stImage.addrVirt.addr1    = virt;
        
        LoadImgFile(img, &stImage);
        
        if(PreProcess(&stImage, &stNet.stNetIn) < 0) {
            printf("[%s|%d] PreProcess failed!", __FUNCTION__, __LINE__);
            break;
        }

        if(Inference(&stNet) < 0) {
            printf("[%s|%d] Inference failed!", __FUNCTION__, __LINE__);
            break;
        }
        
        PostProcess(&stNet, output);
    } while (0);

    if(virt) {
        cavalry_mem_free(size, phys, virt);
        virt = NULL;
    }
    
    AmbaExit(&stNet);

    return 0;
}

void Amba_Inference::usage()
{
    printf("\n");
    printf("**************************************\n");
    printf("需要1个参数: 图片文件名\n");
    printf("示例: ./demo img.jpg\n");
    printf("**************************************");
    printf("\n");
}

std::vector<float> Amba_Inference::amba_pred(cv::Mat img,std::string amba_path)
{
    std::vector<float> output;

//    if (argc != 2) {
//        usage();
//        return -1;
//    }
//
//    Mat img  = cv::imread(argv[1]);
    if (!img.data)
    {
        printf("image [%s] load err!\n");
        exit(-1);
    }

    /* 模型加载运行 */
    Net(img, output);

    return output;
}
