// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/HttpRequest.h"
#include "accelbyte/common/Vector.h"
#include "accelbyte/common/Map.h"
#include "accelbyte/common/MultiMap.h"
#include <utility>

namespace accelbyte {
namespace http {

/**
* @brief base implementation of request that will be used for other class.
**/
class BaseRequest : public HttpRequest {
public:

    /**
    * @brief default constructor.
    **/
    AB_CNL_HTTP_EXPORT BaseRequest() = default;

    /**
    * @brief default detructor.
    **/
    AB_CNL_HTTP_EXPORT virtual ~BaseRequest() = default;

    /**
    * @brief Return the URL the request is to be sent to. 
    *        This include all the parameters encoded properly.
    **/
    AB_CNL_HTTP_EXPORT virtual String url() const override;

    /**
    * @brief return the Method (GET, PUT...) the request is using.
    **/
    AB_CNL_HTTP_EXPORT virtual http_method method() const override;

    /**
    * @brief return the headers this request want to set up.
    *        NB: only headers that are linked to this particular request are to be returned.
    *        Standard header that are system dependant are not to be returned here.
    **/
    AB_CNL_HTTP_EXPORT virtual Map<header_field, header_value> headers() const override;

    /**
     * @brief return the request form.
     **/
    AB_CNL_HTTP_EXPORT virtual Map<String, FormData> form() const override;

protected:

    AB_CNL_HTTP_EXPORT void set_base_url(const String& url);

    using key = String;
    using value = String;
    using int_value = int;
    using vector_value = Vector<String>;
    using int_vector_value = Vector<int>;
    using param_def = std::pair<key, value>;
    using int_param_def = std::pair<key, int_value>;
    using param_vector_def = std::pair<key, vector_value>;
    using int_param_vector_def = std::pair<key, int_vector_value>;
    AB_CNL_HTTP_EXPORT void add_url_param(const param_def& param);
    AB_CNL_HTTP_EXPORT void add_query_param(const param_def& param);
    AB_CNL_HTTP_EXPORT void add_query_param(const int_param_def& param);
    AB_CNL_HTTP_EXPORT void add_query_param(const param_vector_def& param);
    AB_CNL_HTTP_EXPORT void add_query_param(const int_param_vector_def& param);
    AB_CNL_HTTP_EXPORT void add_header_param(const param_def& param);
    AB_CNL_HTTP_EXPORT void add_form_file_param(const param_def& param);
    AB_CNL_HTTP_EXPORT void add_form_param(const param_def& param, bool is_file = false);
    AB_CNL_HTTP_EXPORT void add_form_param(const int_param_def& param);
    AB_CNL_HTTP_EXPORT void add_form_param(const param_vector_def& param);
    AB_CNL_HTTP_EXPORT void add_form_param(const int_param_vector_def& param);

    AB_CNL_HTTP_EXPORT void set_method(http_method method);

private:
    String _base_url;
    http_method _method{http_method::get};
    Map<key, value> _url_params;
    MultiMap<key, value> _query_params;
    Map<key, value> _header_params;
    Map<key, FormData> _form_params;
};

}
}