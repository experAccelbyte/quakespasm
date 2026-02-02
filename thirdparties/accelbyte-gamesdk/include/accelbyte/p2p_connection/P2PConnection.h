// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/p2p_connection/p2p_connection_exports.h"

#include <accelbyte/common/Error.h>
#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace p2p_connection {

/**
 * @brief interface for a P2P connection using
 **/
class AB_CNL_P2P_CONNECTION_EXPORT P2PConnection {
public:
    virtual ~P2PConnection() = default;

    virtual Error get_local_description(String& description) = 0;
    virtual Error set_remote_description(const String& description) = 0;
    virtual Error get_candidates(Vector<String>& candidates) = 0;
    virtual Error get_selected_pair(String& local, String& remote) const = 0;

    virtual Error connect_to_peer(const Vector<String> candidates) = 0;
    virtual void diconnect() = 0;

    virtual bool connected() = 0;

    virtual Error write(const Vector<char>& data) = 0;
    virtual Error write(const char* data, size_t size) = 0;
    virtual Error read(Vector<char>& data) = 0;

    /**
     * @brief wait from data from the websocket
     * @parameter timeout timeout in ms or until data if <= 0
     */
    virtual accelbyte::Error wait_for_data(int timeout = -1) = 0;

    /**
     * @brief stop from data from the websocket
     */
    virtual void stop_waiting() = 0;
};

} // namespace p2p_connection
} // namespace accelbyte