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
 * @brief Basic Authentication header
 *
 */
class BasicAuthentication : public Authentication {
public:
    /**
     * @brief Construct a new Basic Authentication object
     *
     * @param username
     * @param password
     */
    AB_CNL_TLS_EXPORT BasicAuthentication(String username, String password);

    /**
     * @brief set the username
     * 
     * @param username 
     */
    AB_CNL_TLS_EXPORT void username(String username);

    /**
     * @brief set the password
     * 
     * @param password 
     */
    AB_CNL_TLS_EXPORT void password(String password);

    /**
     * @brief get the username
     * 
     * @return const String& 
     */
    AB_CNL_TLS_EXPORT auto username() const -> const String&;

    /**
     * @brief get the password
     * 
     * @return const String& 
     */
    AB_CNL_TLS_EXPORT auto password() const -> const String&;


    /**
     * @brief get the authentication header value
     *
     * @return String
     */
    AB_CNL_TLS_EXPORT auto header_value() const -> String override;

private:
    String username_;
    String password_;
};

} // namespace tls
} // namespace accelbyte