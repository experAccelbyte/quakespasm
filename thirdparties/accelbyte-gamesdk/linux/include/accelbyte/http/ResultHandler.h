// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/common/Error.h"
#include "accelbyte/http/ModelObject.h"

#include <functional>

namespace accelbyte {
namespace http {


/**
* @brief base interfaces to an http request handler.
**/
class AB_CNL_HTTP_EXPORT ResultHandler {
public:
    /**
    * @brief default detructor.
    **/
    virtual ~ResultHandler() = default;

    /**
    * @brief handle request error
    * @param[in] error the error
    **/
    virtual void handle_failure(const Error& er) const = 0;
};

}
}