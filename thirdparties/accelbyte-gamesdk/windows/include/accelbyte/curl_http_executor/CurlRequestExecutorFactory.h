#pragma once

#include "accelbyte/curl_http_executor/curl_http_executor_exports.h"
#include "accelbyte/http/RequestExecutorFactory.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/common/String.h"

namespace accelbyte {
namespace http {

/**
 * @brief a test request executor factory
 **/
class CurlRequestExecutorFactory : public RequestExecutorFactory {
public:
    AB_CNL_CURL_HTTP_EXPORT CurlRequestExecutorFactory() = default;
    AB_CNL_CURL_HTTP_EXPORT ~CurlRequestExecutorFactory() = default;

    /**
     * @brief create an executor
     **/
    AB_CNL_CURL_HTTP_EXPORT virtual memory::SharedPtr<RequestExecutor> make_executor() const override;

    /**
     * @brief add a proxy url
     */
    AB_CNL_CURL_HTTP_EXPORT void set_proxy_url(const String& proxy_url);

private:
    String proxy_url_{};
};

} // namespace http
} // namespace accelbyte
