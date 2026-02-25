// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/HttpRequest.h"
#include "accelbyte/common/Error.h"
#include "accelbyte/memory/memory.h"

namespace accelbyte {
namespace http {

/**
 * @brief error code for the http layer.
 **/
class HttpError : public Error {
public:
    AB_CNL_HTTP_EXPORT HttpError() = delete;
    AB_CNL_HTTP_EXPORT HttpError(
        error_type type,
        const String& code,
        const String& headers,
        const String& body,
        const String& what,
        const HttpRequest& request,
        const void* response_object);
    AB_CNL_HTTP_EXPORT HttpError(const HttpError& other) = delete;
    AB_CNL_HTTP_EXPORT virtual ~HttpError() = default;

    AB_CNL_HTTP_EXPORT HttpError& operator=(const HttpError& other) = delete;

    AB_CNL_HTTP_EXPORT const void* response_object() const;
    AB_CNL_HTTP_EXPORT String url() const;
    AB_CNL_HTTP_EXPORT String method() const;
    AB_CNL_HTTP_EXPORT Map<String, String> headers() const;
    AB_CNL_HTTP_EXPORT String body() const;
    AB_CNL_HTTP_EXPORT String reasons() const;

private:
    const void* response_object_;
    const HttpRequest& request_;
    String reasons_;
};

} // namespace http
} // namespace accelbyte