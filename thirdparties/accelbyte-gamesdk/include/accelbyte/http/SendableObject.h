// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/common/String.h"
namespace accelbyte {
namespace http {

/**
* @brief interfaces that describe an object that can be send through http
**/
class AB_CNL_HTTP_EXPORT SendableObject {
public:

    /**
    * @brief default detructor.
    **/
    virtual ~SendableObject() = default;

    /**
    * @brief Return the object unique id (used for serialization)
    **/
    virtual const String& sendable_id() const  = 0;
};

}
}