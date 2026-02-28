// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/RequestExecutor.h"
#include "accelbyte/memory/memory.h"


namespace accelbyte {
namespace http {


/**
* @brief interfaces to an http request scheduler.
**/
class AB_CNL_HTTP_EXPORT RequestScheduler {
public:
    /**
    * @brief default detructor.
    **/
    virtual ~RequestScheduler() = default;

    /**
    * @brief execute the request using the given executor and return the value in form of raw answer.
    **/
    virtual Error execute_request(
        const memory::SharedPtr < RequestExecutor >& executor, 
        const HttpRequest& request, 
        RawAnswer& answer) const = 0;
};

}
}