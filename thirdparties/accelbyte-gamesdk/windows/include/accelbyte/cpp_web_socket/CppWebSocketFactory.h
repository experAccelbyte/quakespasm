#pragma once

#include "accelbyte/cpp_web_socket/cpp_web_socket_exports.h"
#include "accelbyte/web_socket/WebSocketFactory.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/common/Map.h"
#include "accelbyte/common/String.h"

namespace accelbyte {
namespace cpp_web_socket {

class WebSocketEndPoint;

/**
 * @brief interfaces to an http request scheduler factory.
 **/
class CppWebSocketFactory : public accelbyte::web_socket::WebSocketFactory {
public:
    /**
     * @brief default constructor.
     **/
    AB_CNL_CPP_WEB_SOCKET_EXPORT CppWebSocketFactory();

    /**
     * @brief default detructor.
     **/
    AB_CNL_CPP_WEB_SOCKET_EXPORT virtual ~CppWebSocketFactory() = default;

    /**
     * @brief create a scheduler
     **/
    AB_CNL_CPP_WEB_SOCKET_EXPORT virtual memory::SharedPtr<accelbyte::web_socket::WebSocket> create_socket(
        const memory::SharedPtr<accelbyte::tls::SecurityAuthorization>& authorization,
        const String& url,
        const Map<String, String>& headers) const override;

    /**
     * @brief set the proxy url
     */
    AB_CNL_CPP_WEB_SOCKET_EXPORT void set_proxy_url(const String& proxy_url);

private:
    memory::SharedPtr<WebSocketEndPoint> end_point_;
    String proxy_url_{};
};

} // namespace cpp_web_socket
} // namespace accelbyte