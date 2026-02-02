// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/common/Map.h"
#include "accelbyte/common/String.h"
#include "accelbyte/common/Vector.h"
#include "accelbyte/debug_handler/DebugOutput.h"
#include "accelbyte/debug_handler/debug_handler_exports.h"

#include <memory>
#include <shared_mutex>

namespace Json {
class Value;
}

namespace accelbyte {
namespace debug_handler {

/**
 * @brief interface to a http request executor factory.
 **/
class DebugHandler {
public:
    AB_CNL_DEBUG_HANDLER_EXPORT ~DebugHandler() = default;

    AB_CNL_DEBUG_HANDLER_EXPORT static auto get() -> std::shared_ptr<DebugHandler>;
    /**
     * @brief add a debug output
     **/
    AB_CNL_DEBUG_HANDLER_EXPORT auto add_debug_output(std::shared_ptr<DebugOutput> const& output) -> void;

    /**
     * @brief add a debug output
     **/
    AB_CNL_DEBUG_HANDLER_EXPORT auto log_http_request(
        String const& url,
        String const& action,
        Map<String, String> const& headers,
        String const& body,
        String const& id) -> void;

    /**
     * @brief add a debug output
     **/
    AB_CNL_DEBUG_HANDLER_EXPORT auto log_http_response(
        String const& url,
        String const& action,
        String const& result,
        String const& headers,
        String const& body,
        String const& id) -> void;

private:
    DebugHandler() = default;

    void output_log(const Json::Value& value) const;

    mutable std::mutex mtx_;
    Vector<std::shared_ptr<DebugOutput>> debug_outputs_;
};

} // namespace debug_handler
} // namespace accelbyte