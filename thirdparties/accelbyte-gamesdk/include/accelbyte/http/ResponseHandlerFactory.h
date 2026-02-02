// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/ResponseHandler.h"
#include "accelbyte/http/ResultHandler.h"
#include "accelbyte/http/SendableObject.h"
#include "accelbyte/memory/memory.h"

#include "accelbyte/common/String.h"
#include <unordered_set>

namespace accelbyte {
namespace http {

/**
* @brief access to the request factory
**/
class AB_CNL_HTTP_EXPORT ResponseHandlerFactory {
public:

    /**
    * @brief default destructor.
    **/
    virtual ~ResponseHandlerFactory() = default;

    /**
    * @brief create a request from a sendeable object, if the sendeable object is not managed
    *        by this factory nullptr will be returned
    **/    
    virtual auto create_response_handler(const SendableObject& to_send, const memory::SharedPtr<ResultHandler>& handler) const -> memory::SharedPtr<ResponseHandler> = 0;

    /**
    * @brief a list of the sendable_object ids this factory know to manage 
    **/    
    virtual auto sendable_object_ids() const -> std::unordered_set<String> = 0;

    /**
    * @brief Return the main request factory
    **/    
    static auto get_factory() -> memory::SharedPtr<ResponseHandlerFactory>;
    
    /**
    * @brief Add a sub factory to the request
    **/    
    static void add_sub_factory(memory::SharedPtr<ResponseHandlerFactory>& sub_factory);
};

}
}