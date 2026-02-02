// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#include "crypto_export.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace crypto {
ACCELBYTE_CRYPTO_API auto md5(String const& input) -> String;
} // namespace crypto
} // namespace accelbyte