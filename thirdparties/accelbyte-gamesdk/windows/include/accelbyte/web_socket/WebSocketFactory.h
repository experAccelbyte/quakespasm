// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/tls/SecurityAuthorization.h"
#include "accelbyte/web_socket/WebSocket.h"
#include "accelbyte/web_socket/web_socket_exports.h"
#include "accelbyte/memory/memory.h"

#include "accelbyte/common/String.h"
#include "accelbyte/common/Map.h"

#include <mutex>

namespace accelbyte {
namespace web_socket {

/**
 * @brief interfaces to a websocket factory.
 **/
class AB_CNL_WEB_SOCKET_EXPORT WebSocketFactory {
public:
    /**
     * @brief default detructor.
     **/
    virtual ~WebSocketFactory() = default;

    /**
     * @brief create a web socket
     **/
    virtual memory::SharedPtr<WebSocket> create_socket(
        const memory::SharedPtr<accelbyte::tls::SecurityAuthorization>& authorization,
        const String& url,
        const Map<String, String>& headers) const = 0;

    /**
     * @brief get the last set web socket factory. If none was set return nullptr.
     **/
    static const memory::SharedPtr<WebSocketFactory> get_web_socket_factory();

    /**
     * @brief set web socket to use.
     **/
    static void set_web_socket_factory(const memory::SharedPtr<WebSocketFactory>& factory);

private:
    static std::mutex singleton_mutex;
};

} // namespace web_socket
} // namespace accelbyte