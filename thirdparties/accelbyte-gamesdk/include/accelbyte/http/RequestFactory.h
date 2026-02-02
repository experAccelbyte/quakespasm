// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/common/String.h"
#include "accelbyte/http/HttpRequest.h"
#include "accelbyte/http/SendableObject.h"
#include "accelbyte/http/http_exports.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/tls/SecurityAuthorization.h"

// STL
#include <memory>
#include <unordered_set>

namespace accelbyte {
namespace http {

/**
 * @brief access to the request factory
 **/
class AB_CNL_HTTP_EXPORT RequestFactory {
public:
    /**
     * @brief Default constructor
     */
    RequestFactory() = default;

    /**
     * @brief Default copy constructor
     */
    RequestFactory(RequestFactory const&) = default;

    /**
     * @brief Default move constructor
     */
    RequestFactory(RequestFactory&&) = default;

    /**
     * @brief Default copy assignment
     *
     * @return RequestFactory&
     */
    auto operator=(RequestFactory const&) -> RequestFactory& = default;

    /**
     * @brief Default move assignment
     *
     * @return RequestFactory&
     */
    auto operator=(RequestFactory&&) -> RequestFactory& = default;

    /**
     * @brief default destructor.
     **/
    virtual ~RequestFactory() = default;

    /**
     * @brief create a request from a sendeable object, if the sendeable object is not managed
     *        by this factory nullptr will be returned
     **/
    virtual auto
    create_request(const accelbyte::tls::SecurityAuthorization& authorization, const SendableObject& to_send) const
        -> memory::SharedPtr<HttpRequest> = 0;

    /**
     * @brief a list of the sendable_object ids this factory know to manage
     **/
    virtual auto sendable_object_ids() const -> std::unordered_set<String> = 0;

    /**
     * @brief Return the main request factory
     **/
    static auto get_factory() -> memory::SharedPtr<RequestFactory>;

    /**
     * @brief Add a sub factory to the request
     **/
    static void add_sub_factory(memory::SharedPtr<RequestFactory>& sub_factory);
};

} // namespace http
} // namespace accelbyte