// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/RequestHandler.h"
#include "accelbyte/memory/memory.h"

#include <memory>
#include <mutex>

namespace accelbyte {
namespace http {

/**
 * @brief interface to an http request handler factory.
 **/
class AB_CNL_HTTP_EXPORT RequestHandlerFactory {
public:
    /**
     * @brief default detructor.
     **/
    virtual ~RequestHandlerFactory() = default;

    /**
     * @brief create an handler
     **/
    virtual memory::SharedPtr<RequestHandler> make_handler() const = 0;

    /**
     * @brief get the last set handler factory set. If none was set return a default basic implementation.
     **/
    static memory::SharedPtr<RequestHandlerFactory> get_handler_factory();

    /**
     * @brief set handler factory to use.
     **/
    static void set_handler_factory(memory::SharedPtr<RequestHandlerFactory>& handler);

private:
    static std::mutex singleton_mutex;
};

} // namespace http
} // namespace accelbyte