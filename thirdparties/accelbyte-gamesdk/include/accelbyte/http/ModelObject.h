// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/common/String.h"

namespace accelbyte {
namespace http {

/**
* @brief interfaces that describe an object that can be recieved
**/
class AB_CNL_HTTP_EXPORT ModelObject {
public:
    /**
    * @brief default constructor.
    **/
    ModelObject();
    
    /**
    * @brief copy constructor.
    **/
    ModelObject(const ModelObject& other);

    /**
    * @brief default detructor.
    **/
    virtual ~ModelObject();

    /**
    * @brief Return the object unique id (used for serialization)
    **/
    virtual const String& model_id() const  = 0;
};

}
}