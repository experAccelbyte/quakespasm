// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/HttpRequest.h"
#include "accelbyte/http/ResponseHandler.h"
#include "accelbyte/http/http_exports.h"
#include "accelbyte/memory/memory.h"

namespace accelbyte {
namespace http {

/**
 * @brief interface to a task
 **/
class Task {
public:
    /**
     * @brief default constructor.
     **/
    Task() = delete;

    AB_CNL_HTTP_EXPORT
    Task(const memory::SharedPtr<HttpRequest>& request, const memory::SharedPtr<ResponseHandler>& handler);

    /**
     * @brief default detructor.
     **/
    AB_CNL_HTTP_EXPORT ~Task() = default;

    /**
     * @brief Return the request to be sent
     **/
    AB_CNL_HTTP_EXPORT const HttpRequest& get_request() const;

    /**
     * @brief return the response handler.
     **/
    AB_CNL_HTTP_EXPORT const ResponseHandler& get_response_handler() const;

private:
    memory::SharedPtr<HttpRequest> _request;
    memory::SharedPtr<ResponseHandler> _handler;
};

} // namespace http
} // namespace accelbyte