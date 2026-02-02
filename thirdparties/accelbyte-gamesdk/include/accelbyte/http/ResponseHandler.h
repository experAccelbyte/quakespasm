// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/HttpRequest.h"
#include "accelbyte/common/Error.h"
#include "accelbyte/memory/memory.h"

#include "accelbyte/common/Map.h"
#include "accelbyte/common/Vector.h"
#include "accelbyte/common/String.h"


namespace accelbyte {
namespace http {

/**
* @brief interfaces to an http answer handler.
**/
class AB_CNL_HTTP_EXPORT ResponseHandler {
public:
    /**
    * @brief default detructor.
    **/
    virtual ~ResponseHandler() = default;

    /**
    * @brief handle the answer from a server
    * @param code the ansver code, including description
    * @param headers the response headers
    * @param body the response body
    **/
    virtual void handle_response(
        const String& code, 
        const String& headers,
        const String& body, 
        const HttpRequest& request) const = 0;

    virtual void handle_failure(const Error& ec) const = 0;
};

}
}