#pragma once

#include "accelbyte/debug_http_executor/debug_http_executor_exports.h"
#include "accelbyte/http/RequestExecutorFactory.h"
#include "accelbyte/memory/memory.h"

namespace accelbyte {
namespace http {

/**
 * @brief a test request executor factory
 **/
class DebugRequestExecutorFactory : public RequestExecutorFactory {
public:
    AB_CNL_DEBUG_HTTP_EXPORT DebugRequestExecutorFactory() = default;
    AB_CNL_DEBUG_HTTP_EXPORT DebugRequestExecutorFactory(bool verbose, bool duration);
    AB_CNL_DEBUG_HTTP_EXPORT  ~DebugRequestExecutorFactory() = default;

    /**
     * @brief create an executor
     **/
    AB_CNL_DEBUG_HTTP_EXPORT virtual memory::SharedPtr<RequestExecutor> make_executor() const override;

    /**
     * @brief add a custom header to the next request.
     **/
    AB_CNL_DEBUG_HTTP_EXPORT void add_custom_header(const String& key, const String& value);

    /**
     * @brief add credential to the next request.
     **/
    AB_CNL_DEBUG_HTTP_EXPORT void add_credential(const String& id, const String& pwd);

private:
    mutable Map<String, String> custom_headers_;
    mutable String id_;
    mutable String pwd_;
    bool verbose_{true};
    bool duration_{true};
};

} // namespace http
} // namespace accelbyte

