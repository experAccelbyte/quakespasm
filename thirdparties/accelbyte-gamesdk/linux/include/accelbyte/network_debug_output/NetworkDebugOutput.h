// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/common/String.h"
#include "accelbyte/debug_handler/DebugOutput.h"
#include "accelbyte/network_debug_output/network_debug_output_exports.h"

// STL
#include <mutex>

namespace accelbyte {
namespace network_utility {
class NetworkUtility;
}

namespace network_debug_output {

/**
 * @brief interface to a debug output
 **/
class NetworkDebugOutput : public debug_handler::DebugOutput {
public:
    AB_CNL_NETWORK_DEBUG_OUTPUT_EXPORT NetworkDebugOutput(String const& address, int16_t port);

    /**
     * @brief default detructor.
     **/
    AB_CNL_NETWORK_DEBUG_OUTPUT_EXPORT ~NetworkDebugOutput() override;

    /**
     * @brief execute the request and return the value in form of raw answer. Should handle 300 code
     **/
    AB_CNL_NETWORK_DEBUG_OUTPUT_EXPORT void output_debug(String const& output) const override;

private:
    bool connect() const;

    mutable memory::SharedPtr<network_utility::NetworkUtility> connection_;
    mutable bool connected_{false};
    mutable std::mutex mtx_;

    String address_;
    int16_t port_;
};

} // namespace network_debug_output
} // namespace accelbyte