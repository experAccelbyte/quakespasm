// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/common/String.h"
#include "accelbyte/http/http_exports.h"
#include "accelbyte/memory/memory.h"
namespace accelbyte {
namespace http {

/**
 * @brief a raw http answer.
 **/
struct RawAnswer {
    AB_CNL_HTTP_EXPORT RawAnswer() = default;
    AB_CNL_HTTP_EXPORT RawAnswer(RawAnswer const&) = default;
    AB_CNL_HTTP_EXPORT RawAnswer(RawAnswer&&) = default;
    AB_CNL_HTTP_EXPORT auto operator=(RawAnswer&&) -> RawAnswer& = default;
    AB_CNL_HTTP_EXPORT auto operator=(RawAnswer const&) -> RawAnswer& = default;

    AB_CNL_HTTP_EXPORT RawAnswer(String lcode, String lheaders, String lbody)
        : code(std::move(lcode))
        , headers(std::move(lheaders))
        , body(std::move(lbody))
    {
    }

    AB_CNL_HTTP_EXPORT ~RawAnswer() = default;
    String code;
    String headers;
    String body;
};

} // namespace http
} // namespace accelbyte