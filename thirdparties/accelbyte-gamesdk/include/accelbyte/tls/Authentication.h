// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/tls/tls_exports.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/common/String.h"

namespace accelbyte {
namespace tls {

/**
 * @brief interface for HTTP authentication header.
 **/
class AB_CNL_TLS_EXPORT Authentication {
public:
    virtual ~Authentication() = default;

    /**
     * @brief get the authentication header value
     *
     * @return String
     */
    virtual auto header_value() const -> String = 0;
};

} // namespace tls
} // namespace accelbyte