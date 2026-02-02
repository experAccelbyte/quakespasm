// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/common/String.h"
#include "accelbyte/common/Map.h"


namespace accelbyte {
namespace http {


class AB_CNL_HTTP_EXPORT FormData {
public:
    FormData(const String& data, const bool& is_file = false);
    auto data() const -> const String&;
    auto is_file() const -> const bool&;

private:
    String data_;
    bool is_file_;
};

}
}