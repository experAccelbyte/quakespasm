// Copyright (c)  AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "accelbyte/codegen_common/SerializerFactory.h"
#include "accelbyte/common/Map.h"
#include "accelbyte/common/Vector.h"
#include "accelbyte/utils/JsonObjectString.h"
#include "accelbyte/http/BaseRequest.h"
#include "accelbyte/http/ModelObject.h"
#include "codegen_common_export.h"

namespace accelbyte {
namespace common {

class ACCELBYTE_CODEGEN_COMMON_API CodegenBaseRequest : public accelbyte::http::BaseRequest {

public:
    /**
    * @brief default constructor.
    **/
    CodegenBaseRequest() = default;
	
protected:
    virtual SerializerFactory& get_serializer_factory() = 0;
	
    void do_serialize(const accelbyte::http::ModelObject& from, String& to);
    void do_serialize(const String& from, String& to);
    void do_serialize(const long& from, String& to);
    void do_serialize(const Map<String, utils::JsonObjectString>& from, String& to);
    void do_serialize(const Vector<utils::JsonObjectString>& from, String& to);
};

}
}