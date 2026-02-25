// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/RequestScheduler.h"
#include "accelbyte/memory/memory.h"

#include <mutex>

namespace accelbyte {
namespace http {

/**
 * @brief interfaces to an http request scheduler factory.
 **/
class AB_CNL_HTTP_EXPORT RequestSchedulerFactory {
public:
    /**
     * @brief default detructor.
     **/
    virtual ~RequestSchedulerFactory() = default;

    /**
     * @brief create a scheduler
     **/
    virtual memory::SharedPtr<RequestScheduler> make_scheduler() const = 0;

    /**
     * @brief get the last set scheduler factory. If none was set return a default implementation.
     **/
    static memory::SharedPtr<RequestSchedulerFactory> get_scheduler_factory();

    /**
     * @brief set scheduler to use.
     **/
    static void set_scheduler_factory(memory::SharedPtr<RequestSchedulerFactory>& scheduler);

private:
    static std::mutex singleton_mutex;
};

} // namespace http
} // namespace accelbyte