// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/debug_handler/debug_handler_exports.h"
#include "accelbyte/common/String.h"

namespace accelbyte {
namespace debug_handler{

/**
 * @brief interface to a debug output
 **/
class AB_CNL_DEBUG_HANDLER_EXPORT DebugOutput {
public:
    /**
     * @brief default detructor.
     **/
    virtual ~DebugOutput() = default;

    /**
     * @brief execute the request and return the value in form of raw answer. Should handle 300 code
     **/
    virtual void output_debug(const String& output) const = 0;
};

} // namespace http
} // namespace accelbyte