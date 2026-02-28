// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/FormData.h"
#include "accelbyte/common/String.h"
#include "accelbyte/common/Map.h"


namespace accelbyte {
namespace http {

/**
* @brief interface that a request must comply with in order to be sent.
**/
class AB_CNL_HTTP_EXPORT HttpRequest {
public:
    enum class http_method {
        get,
        post,
        patch,
        put,
        delete_
    };

    /**
    * @brief default detructor.
    **/
    virtual ~HttpRequest() = default;

    /**
    * @brief Return the URL the request is to be sent to. 
    *        This include all the parameters encoded properly.
    **/
    virtual String url() const  = 0;

    /**
    * @brief return the Method (GET, PUT...) the request is using.
    **/
    virtual http_method method() const = 0;

    /**
     * @brief return the Method (GET, PUT...) the request is using.
     **/
    String method_string() const
    {
        switch (method()) {
        case http_method::get:
            return "GET";
        case http_method::patch:
            return "PATCH";
        case http_method::post:
            return "POST";
        case http_method::put:
            return "PUT";
        case http_method::delete_:
            return "DELETE";
        default:
            return "UNKNOWN";
        }
    }

    /**
    * @brief return the headers this request want to set up.
    *        NB: only headers that are linked to this particular request are to be returned.
    *        Standard header that are system depenndant are not to be returned here.
    **/
    using header_field = String;
    using header_value = String;
    virtual Map<header_field, header_value> headers() const = 0;

    /**
    * @brief return the request body.
    **/
    virtual String body() const = 0;

    /**
     * @brief return the request form.
     **/
    virtual Map<String, FormData> form() const = 0;
};

}
}