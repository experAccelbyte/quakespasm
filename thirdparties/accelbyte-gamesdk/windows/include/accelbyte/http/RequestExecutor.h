// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/common/Error.h"
#include "accelbyte/http/HttpRequest.h"
#include "accelbyte/http/RawAnswer.h"
#include "accelbyte/http/http_exports.h"

namespace accelbyte {
namespace http {

/**
 * @brief interface to an http request executor.
 **/
class AB_CNL_HTTP_EXPORT RequestExecutor {
public:
    /**
     * @brief default detructor.
     **/
    virtual ~RequestExecutor() = default;

    /**
     * @brief execute the request and return the value in form of raw answer. Should handle 300 code
     **/
    virtual Error execute_request(const HttpRequest& request, RawAnswer& answer) const = 0;

    /**
     * @brief set the proxy url
     */
    virtual void set_proxy_url(const String& proxy_url) = 0;
};

} // namespace http
} // namespace accelbyte