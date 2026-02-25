// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/tls/tls_exports.h"
#include "accelbyte/memory/memory.h"
#include "Authentication.h"

#include "accelbyte/common/String.h"

namespace accelbyte {
namespace tls {

/**
 * @brief Bearer Authentication header
 *
 */
class BearerAuthentication : public Authentication {
public:
    /**
     * @brief Construct a new Bearer Authentication object
     *
     * @param token
     */
    AB_CNL_TLS_EXPORT BearerAuthentication(String token);

    /**
     * @brief set the token
     *
     * @param token
     */
    AB_CNL_TLS_EXPORT void token(String token);

    /**
     * @brief get the token
     *
     * @return const String&
     */
    AB_CNL_TLS_EXPORT auto token() const -> const String&;

    /**
     * @brief get the authentication header value
     *
     * @return String
     */
    AB_CNL_TLS_EXPORT auto header_value() const -> String override;

private:
    String token_;
};

} // namespace tls
} // namespace accelbyte