// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "user_exports.h"

#include "Token.h"

namespace accelbyte {
namespace user {

/**
 * @brief Token update listener
 *
 */
class TokenUpdateListener {
public:
    ~TokenUpdateListener(){};

    virtual auto on_token_updated(const Token& token) -> void = 0;
};

} // namespace ags
} // namespace accelbyte