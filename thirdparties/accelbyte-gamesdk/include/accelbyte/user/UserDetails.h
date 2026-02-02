// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "user_exports.h"
#include "User.h"

// STL
#include <accelbyte/common/String.h>
#include <accelbyte/common/Error.h>
#include <accelbyte/iam/models/AdminUserV3.h>

namespace accelbyte {
namespace user {

class UserDetails {
public:
    ACCELBYTE_USER_API static auto request(
        const memory::SharedPtr<accelbyte::user::User>& credential,
        std::function<void(memory::SharedPtr<iam::model::AdminUserV3>&)> on_success,
        std::function<void(const Error&)> on_error) -> void;
};

} // namespace ags
} // namespace accelbyte