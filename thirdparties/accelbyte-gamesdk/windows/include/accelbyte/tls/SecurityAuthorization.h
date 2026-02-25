// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#include "accelbyte/tls/Authentication.h"
#include "accelbyte/memory/memory.h"

namespace accelbyte {
namespace tls {

/**
 * @brief inteface for HTTP authorization header.
 **/
class SecurityAuthorization {
public:

    virtual ~SecurityAuthorization() = default;

    /**
     * @brief get the basic authentication
     *
     * @return the authentication object
     */
    virtual auto basic_authentication() const -> memory::SharedPtr<Authentication> = 0;

    /**
     * @brief get the bearer authentication
     * 
     * @return the authentication object
     */
    virtual auto bearer_authentication() const -> memory::SharedPtr<Authentication> = 0;

    /**
     * @brief get the user namespace
     *
     * @return the user namespace
     */
    virtual auto user_namespace() const -> const String& = 0;
};

} // namespace tls
} // namespace accelbyte