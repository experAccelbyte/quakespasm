// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/HttpRequest.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/test_utils/test_utils_exports.h"

namespace accelbyte {
namespace test_utils {

/**
 * @brief interfaces that a request must comply with in order to be sent.
 * @rfc I set the return parameter to not be reference because I don't want to force implementation
 * to keep a particular parameter in a given format.
 **/
class TestRequest : public accelbyte::http::HttpRequest {
public:
    /**
     * @brief default constructor.
     **/
    TestRequest() = delete;

    AB_CNL_TESTS_UTILS_EXPORT TestRequest(
        const String& url,
        const http_method method,
        const Map<header_field, header_value>& headers,
        const String& body);

    AB_CNL_TESTS_UTILS_EXPORT TestRequest(const TestRequest& other);

    /**
     * @brief default detructor.
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual ~TestRequest() = default;

    /**
     * @brief Return the URL the request is to be sent to.
     *        This include all the parameters encoded properly.
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual String url() const override;

    /**
     * @brief return the Method (GET, PUT...) the request is using.
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual http_method method() const override;

    /**
     * @brief return the headers this request want to set up.
     *        NB: only headers that are linked to this particular request are to be returned.
     *        Standard header that are system depenndant are not to be returned here.
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual Map<header_field, header_value> headers() const override;

    /**
     * @brief return the request body.
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual String body() const override;

private:
    String _url;
    http_method _method;
    Map<header_field, header_value> _headers;
    String _body;
};

} // namespace test_utils
} // namespace accelbyte