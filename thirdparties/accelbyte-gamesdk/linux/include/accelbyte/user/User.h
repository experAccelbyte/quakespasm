// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "user_exports.h"
#include "UserCredential.h"
#include <accelbyte/iam/models/AdminUserV3.h>

//
#include "accelbyte/common/DateTime.h"
#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace user {

class User {
public:
    /**
     * @brief Construct a new User Credential object
     *
     */
    ACCELBYTE_USER_API User();

    /**
     * @brief Construct a new User Credential object
     *
     * @param access_token
     * @param refresh_token
     */
    ACCELBYTE_USER_API User(const memory::SharedPtr<UserCredential>& user_credential);

    
    ACCELBYTE_USER_API ~User();

    ACCELBYTE_USER_API operator const accelbyte::tls::SecurityAuthorization&() const;
    ACCELBYTE_USER_API auto credential() const -> const memory::SharedPtr<UserCredential>&;
    ACCELBYTE_USER_API auto user_data() const -> const memory::SharedPtr<iam::model::AdminUserV3>&;
    ACCELBYTE_USER_API auto user_id() const -> const String&;
    ACCELBYTE_USER_API auto display_name() const -> const String&;
    ACCELBYTE_USER_API auto platform_id() const -> const Optional<String>&;
    ACCELBYTE_USER_API auto platform_user_id() const -> const Optional<String>&;

    /**
     * @brief Set the user details.
     *
     * @param details The new user details to be assigned.
     */
    ACCELBYTE_USER_API void set_user_details(const memory::SharedPtr<iam::model::AdminUserV3>& details);

private:

    memory::SharedPtr<UserCredential> user_credential_{};
    memory::SharedPtr<iam::model::AdminUserV3> user_details_{};
};

} // namespace ags
} // namespace accelbyte