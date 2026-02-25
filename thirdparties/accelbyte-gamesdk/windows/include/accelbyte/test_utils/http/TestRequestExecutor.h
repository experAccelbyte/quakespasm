// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/RequestExecutor.h"
#include "accelbyte/test_utils/test_utils_exports.h"

namespace accelbyte {
namespace test_utils {

/**
 * @brief interfaces that a request must comply with in order to be sent.
 * @rfc I set the return parameter to not be reference because I don't want to force implementation
 * to keep a particular parameter in a given format.
 **/
class TestRequestExecutor : public accelbyte::http::RequestExecutor {
public:
    /**
     * @brief default constructor.
     **/
    TestRequestExecutor() = delete;

    /**
     * @brief default constructor.
     **/
    AB_CNL_TESTS_UTILS_EXPORT TestRequestExecutor(const accelbyte::http::RawAnswer& answer, accelbyte::Error ec);

    /**
     * @brief execute the request and return the value in form of raw answer.
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual accelbyte::Error
    execute_request(const accelbyte::http::HttpRequest& request, accelbyte::http::RawAnswer& answer) const override;

        /**
         * @brief set the proxy url
         */
        void set_proxy_url(const accelbyte::String& proxy_url) override;

    private:
        accelbyte::http::RawAnswer answer_;
        accelbyte::Error ec_;
    };

} // namespace test_utils
} // namespace accelbyte