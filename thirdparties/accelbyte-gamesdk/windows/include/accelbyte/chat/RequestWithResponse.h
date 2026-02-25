// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "chat_exports.h"
#include "Request.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {

template<class RspType>
class RequestWithResponse : public Request {
public:
    typedef RspType ResponseType;
    static_assert(std::is_base_of<Response, RspType>::value, "RspType must be derived from Response");
};

}
}