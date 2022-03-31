#pragma once

#include <cinttypes>
#include <atomic>

#include "ialg_error_code.h"
#include "ialg_open_app.h"

#include "task_queue.h"

class analyser
{
public:
    static analyser &instance()
    {
        static analyser als;
        return als;
    }

    void registerPrintCb(const LOG_FUNC_CB pfn)
    {
        m_printCb = pfn;
    }

    void registerOdCb(const IALG_OD_ANALYZE_RESULT_CB pfn)
    {
        m_odCb = pfn;
    }

    void registerOaCb(const IALG_ANALYZE_RESULT_CB_V1 pfn)
    {
        m_oaCb = pfn;
    }

    void pushOdResult(int32_t iChannelNo, const IALG_OD_RESULT_S *pstOdResult, const IALG_IMAGE_INFO_S *pstImage)
    {
        if (m_odCb)
        {
            m_odCb(iChannelNo, pstOdResult, pstImage);
        }
    }

    void pushOaResult(int32_t iChannelNo, const IALG_REG_RST_S *pstRegRsts,
                      const IALG_IMAGE_INFO_S *pstImage, void *pPrivateData)
    {
        if (m_oaCb)
        {
            m_oaCb(iChannelNo, pstRegRsts, pstImage, pPrivateData);
        }
    }

    bool addTask(const std::function<void()> &f)
    {
        if (m_spQueue)
        {
            return m_spQueue->tryPush(f);
        }
        return false;
    }
    void initQueue(size_t n)
    {
        m_spQueue = std::make_shared<taskQueue>(n);
    }

private:
    IALG_OD_ANALYZE_RESULT_CB m_odCb{nullptr};
    IALG_ANALYZE_RESULT_CB_V1 m_oaCb{nullptr};
    LOG_FUNC_CB m_printCb{nullptr};

    std::shared_ptr<taskQueue> m_spQueue{nullptr};
};
