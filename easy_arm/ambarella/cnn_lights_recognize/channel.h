#pragma once

#include <cinttypes>
#include <map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <functional>
#include <string>

#include <json11.hpp>

#include "ctx.h"

namespace Detail
{
class AlgoChannel
{
public:
    explicit AlgoChannel(uint32_t chnNo) : m_chnNo{ chnNo } {}
    ~AlgoChannel() = default;

    int32_t setConfig(const char *pcConfigJson)
    {
        if (pcConfigJson)
        {
            Ctx::instance().log()(0, "set chn:%u config:%s", m_chnNo, pcConfigJson);

            std::string err;
            auto js = json11::Json::parse(pcConfigJson, err);
            m_eventSort = js["event_sort"].int_value();
            m_threshold = js["threshold"].int_value();

            m_configStr = pcConfigJson;
            return IALG_OK;
        }
        return IALG_NOK;
    }

    int32_t putFrame(const IALG_IMAGE_INFO_S *pstImgInfo, IALG_FREE_IMAGE_INFO_MEM pfnFreeMem, void *pPrivateData)
    {
        static uint32_t objId{0};
        auto rc = Ctx::instance().addTask([=]() {
            // 此处对图像进行分析

            // 根据分析结果填充结果字段
            json11::Json resultJson = json11::Json::object{
                {"eventSort", (int)(m_eventSort)},
                {"behaviorBeginTime", "20210101121212"},
                {"behaviorEndTime", "20210101121213"},
                {"posX", (int)(100)},
                {"posY", (int)(200)},
                {"width", (int)(300)},
                {"height", (int)(400)},
                
                {"resultData1", (int)(m_chnNo)},
                {"resultData2", "字符串结果"},
                {"is_detect", true},
                };
            auto jsonstr = resultJson.dump();

            auto spOaRes = std::make_shared<IALG_REG_RST_S>();
            spOaRes->pcResultJson = const_cast<char *>(jsonstr.c_str());

            IALG_OBJ_IMAGE_S ImageInfo;
            ImageInfo.uiLftX = 100;
            ImageInfo.uiLftY = 200;
            ImageInfo.uiWidth = 300;
            ImageInfo.uiHeight = 400;

            auto &objInfo = spOaRes->stObjInfo;
            objInfo.pstObjImageInfo = &ImageInfo;
            objInfo.uiObjImageNum = 1;
            objInfo.uiObjID = ++objId;
            objInfo.enObjType = IALG_IMAGE_OBJ;

            Ctx::instance().pushOaResult(m_chnNo, spOaRes.get(), pstImgInfo, pPrivateData);

            if (pfnFreeMem)
            {
                pfnFreeMem(pstImgInfo, pPrivateData);
            }
        });

        if (false == rc)
        {
            if (pfnFreeMem)
            {
                pfnFreeMem(pstImgInfo, pPrivateData);
            }
            return IALG_NOK;
        }
        return IALG_OK;
    }

private:
    uint32_t m_chnNo{0};
    std::string m_configStr;
    int32_t m_eventSort{0};
    int32_t m_threshold{0};
};

class ChannelManager
{
public:
    explicit ChannelManager(uint32_t maxSupportChn)
    {
        // 我们设定一个key对应的value是nullptr时表示这个通道可用
        // 创建通道号到通道的映射
        // key为[0, maxSupportChn)
        {
            std::lock_guard<std::mutex> lg(m_chns_mtx);
            for (auto i = 0U; i != maxSupportChn; ++i)
            {
                m_chns.emplace(i, nullptr);
            }
        }
    }
    ~ChannelManager() = default;

    int32_t allocChn()
    {
        std::lock_guard<std::mutex> lg(m_chns_mtx);
        auto it = std::find_if(m_chns.begin(), m_chns.end(), [](const decltype(m_chns)::value_type &elem) {
            return nullptr == elem.second; //value是nullptr时表示这个通道可用
        });

        if (m_chns.end() == it)
        {
            return -1;
        }
        else
        {
            // 通道可用时,创建这个通道的数据结构,并且返回对应的通道号
            it->second = std::make_shared<AlgoChannel>(it->first);
            return it->first;
        }
    }

    void releaseChn(uint32_t chnNo)
    {
        std::lock_guard<std::mutex> lg(m_chns_mtx);
        auto it = m_chns.find(chnNo);
        if (m_chns.end() != it)
        {
            it->second = nullptr;
        }
    }

    int32_t channalHandle(uint32_t chnNo, const std::function<int32_t(AlgoChannel &)> &cbk)
    {
        std::lock_guard<std::mutex> lg(m_chns_mtx);
        auto it = m_chns.find(chnNo);
        if (m_chns.end() != it && nullptr != it->second)
        {
            return cbk(*it->second);
        }
        return IALG_NOK;
    }

private:
    std::mutex m_chns_mtx;
    std::map<uint32_t, std::shared_ptr<AlgoChannel>> m_chns;
};

}; // namespace Detail