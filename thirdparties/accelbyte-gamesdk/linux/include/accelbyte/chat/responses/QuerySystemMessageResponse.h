// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Response.h"
#include "../chat_exports.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>
#include <accelbyte/utils/JsonObjectString.h>

namespace accelbyte {
namespace chat {
namespace responses {

class QuerySystemMessageResponse : public Response {
public:
    struct SystemMessageData {
        String id{};
        String category{};
        String message{};
        String created_at{};
        String updated_at{};
        String expired_at{};
        String read_at{};
    };

    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API QuerySystemMessageResponse();
    ACCELBYTE_CHAT_API virtual ~QuerySystemMessageResponse();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String processed{};
    Vector<SystemMessageData> data{};
};

} // namespace responses
} // namespace chat
} // namespace accelbyte
