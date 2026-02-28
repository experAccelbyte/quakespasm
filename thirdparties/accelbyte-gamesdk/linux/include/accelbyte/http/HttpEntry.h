// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/SendableObject.h"
#include "accelbyte/http/ResultHandler.h"
#include "accelbyte/tls/SecurityAuthorization.h"
#include "accelbyte/memory/memory.h"

namespace accelbyte {
namespace http {
    
    void AB_CNL_HTTP_EXPORT send(
        const accelbyte::tls::SecurityAuthorization& authorization, 
        const SendableObject& to_send, 
        const memory::SharedPtr<ResultHandler>& handler);    
}
}