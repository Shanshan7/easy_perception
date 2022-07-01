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
#include "image.h"
#include "traffic_lights_classifier.h"

static Image image_process;
static TrafficLightsClassifier traffic_lights_classifier;

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
            json11::Json::array dect_region_array = js["algo_area"].array_items();
            dect_region_vector.resize(dect_region_array.size());
            for (int i = 0; i < dect_region_array.size(); i++) 
            {
                json11::Json::array tmp = dect_region_array[i]["points"].array_items();
                dect_region_vector[i].resize(tmp.size());
                dect_region_vector[i][0] = tmp[0]["x"].int_value();
                dect_region_vector[i][1] = tmp[0]["y"].int_value();
                dect_region_vector[i][2] = tmp[2]["x"].int_value() - tmp[0]["x"].int_value();
                dect_region_vector[i][3] = tmp[2]["y"].int_value() - tmp[0]["y"].int_value();
            }             
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
            IALG_IMAGE_FORMAT_E enImageFormat = pstImgInfo->enImageFormat;
            Ctx::instance().log()(3, "IALG_IMAGE_FORMAT_E: %d", enImageFormat);

            cv::Mat input_image;
            image_process.IALG_to_mat(pstImgInfo, input_image);
            Ctx::instance().log()(3, "input_image_shape: %d %d %d", input_image.rows, input_image.cols, input_image.channels());

            traffic_lights_classifier.traffic_lights_results.clear();

            // cv::imwrite("/usr/app/bin/result.png", input_image);
            // Ctx::instance().log()(3, "[%s:%d] save image complete", __func__, __LINE__);

            for (int i = 0; i < dect_region_vector.size(); i++)
            {
                std::vector<float> traffic_lights_locations = {20, 20, 100, 100};
                // traffic_lights_classifier.red_green_yellow(input_image, dect_region_vector[i], m_threshold);
                traffic_lights_classifier.red_green_yellow(input_image, traffic_lights_locations, m_threshold);
            }
            // for (int k = 0; k < traffic_lights_classifier.traffic_lights_results.size(); k++)
            // {
            //     Ctx::instance().log()(3, "size: %d, target_id: %d, target_type: %d",  
            //                              traffic_lights_classifier.traffic_lights_results.size(),
            //                              traffic_lights_classifier.traffic_lights_results[k].target_id,
            //                              traffic_lights_classifier.traffic_lights_results[k].traffic_lights_type);
            // }

            // 根据分析结果填充结果字段
            json11::Json resultJson = json11::Json::object{
                {"eventSort", (int)(m_eventSort)},
                {"behaviorBeginTime", "20210101121212"},
                {"behaviorEndTime", "20210101121213"},
                {"posX", (int)(100)},
                {"posY", (int)(200)},
                {"width", (int)(300)},
                {"height", (int)(400)},
                
                {"target_id", (int)traffic_lights_classifier.traffic_lights_results[0].target_id},
                {"target_type", (int)traffic_lights_classifier.traffic_lights_results[0].traffic_lights_type},
                {"target_rect_height", (int)traffic_lights_classifier.traffic_lights_results[0].traffic_lights_location[2]},
                {"target_rect_width", (int)traffic_lights_classifier.traffic_lights_results[0].traffic_lights_location[3]},
                {"target_rect_x", (int)traffic_lights_classifier.traffic_lights_results[0].traffic_lights_location[0]},
                {"target_rect_y", (int)traffic_lights_classifier.traffic_lights_results[0].traffic_lights_location[1]},
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
    std::vector<std::vector<float>> dect_region_vector;
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