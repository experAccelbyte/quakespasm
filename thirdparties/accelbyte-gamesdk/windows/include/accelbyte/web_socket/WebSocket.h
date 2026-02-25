// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/web_socket/web_socket_exports.h"
#include "accelbyte/common/Error.h"
#include "accelbyte/memory/memory.h"

#include "accelbyte/common/Vector.h"

namespace accelbyte {
namespace web_socket {

/**
 * @brief interface for a web socket
 **/
class AB_CNL_WEB_SOCKET_EXPORT WebSocket {
public:
    /**
    * @brief try and connect the websocket and return error code if failed
    */
    virtual accelbyte::Error connect() = 0;

    /**
     * @brief graciously disconnect a web socket
     */
    virtual void disconnect() = 0;

    /**
     * @brief write a bunch of data to a web socket
     */
    virtual accelbyte::Error write(const Vector<char>& buffer) = 0;

    /**
     * @brief read data from the web socket
     */
    virtual accelbyte::Error read(Vector<char>& buffer) = 0;

    /**
     * @brief wait from data from the websocket
     * @parameter timeout timeout in ms or until data if <= 0
     */
    virtual accelbyte::Error wait_for_data(int timeout = -1) = 0;

    /**
     * @brief stop from data from the websocket
     */
    virtual void stop_waiting() = 0;

    /**
     * @brief return the connected state of the websocket
     */
    virtual bool is_connected() = 0;

    /**
     * @brief add or update a header to the websocket
     */
    virtual void add_header(const String& key, const String& value) = 0;

    /**
     * @brief remove a header to the websocket
     */
    virtual void remove_header(const String& key) = 0;

    /**
     * @brief set the proxy url
     */
    virtual void set_proxy_url(const String& proxy_url) = 0;
};

} // namespace web_socket
} // namespace accelbyte