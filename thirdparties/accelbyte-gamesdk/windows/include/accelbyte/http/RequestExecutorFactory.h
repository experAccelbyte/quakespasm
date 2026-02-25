// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/RequestExecutor.h"
#include "accelbyte/memory/memory.h"

#include <memory>
#include <mutex>

namespace accelbyte {
namespace http {

/**
 * @brief interface to a http request executor factory.
 **/
class AB_CNL_HTTP_EXPORT RequestExecutorFactory {
public:
    /**
     * @brief default detructor.
     **/
    virtual ~RequestExecutorFactory() = default;

    /**
     * @brief create an executor
     **/
    virtual memory::SharedPtr<RequestExecutor> make_executor() const = 0;

    /**
     * @brief get the last set executor factory if none was set return nullptr.
     **/
    static const memory::SharedPtr<RequestExecutorFactory> get_executor_factory();

    /**
     * @brief set executor factory to use.
     **/
    static void set_executor_factory(const memory::SharedPtr<RequestExecutorFactory>& executor);

private:
    static std::mutex singleton_mutex;
};

} // namespace http
} // namespace accelbyte