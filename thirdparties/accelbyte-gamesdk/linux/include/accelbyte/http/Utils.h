// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "http_exports.h"
#include <accelbyte/common/String.h>


namespace accelbyte {
namespace http {

AB_CNL_HTTP_EXPORT String url_encode(const String& value);

} // namespace http
} // namespace accelbyte