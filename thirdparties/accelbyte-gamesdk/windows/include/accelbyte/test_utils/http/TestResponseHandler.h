// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/ResponseHandler.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/test_utils/test_utils_exports.h"

#include <functional>

namespace accelbyte {
namespace test_utils {

/**
 * @brief interfaces to an http answer handler.
 **/
class TestResponseHandler : public accelbyte::http::ResponseHandler {
public:
    TestResponseHandler() = delete;
    AB_CNL_TESTS_UTILS_EXPORT
    TestResponseHandler(
        const std::function<void()>& on_sucess, const std::function<void(const accelbyte::Error&)>& on_error);
    AB_CNL_TESTS_UTILS_EXPORT TestResponseHandler(const TestResponseHandler& other);

    /**
     * @brief default detructor.
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual ~TestResponseHandler() = default;

    /**
     * @brief handle the answer from a server
     * @param code the ansver code, including description
     * @param headers the response headers
     * @param body the response body
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual void
    handle_response(const String& code, const String& headers, const String& body, const http::HttpRequest&) const override;

    AB_CNL_TESTS_UTILS_EXPORT virtual void handle_failure(const accelbyte::Error& ec) const override;

private:
    std::function<void()> on_sucess_;
    std::function<void(const accelbyte::Error&)> on_error_;
};

} // namespace test_utils
} // namespace accelbyte