#pragma once

#include "accelbyte/common/exports.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/common/String.h"

namespace accelbyte {

/**
 * @brief generic error.
 **/
class  Error {

public:
    enum error_type{
        ok,
        timeout,
        cant_reach,
        missing_executor,
        unknown_result_handler,
        client_error,
        server_error,
        unknown_answer,
        unknown_answer_type,
        unknown_error
    };

    ACCELBYTE_COMMON_API Error();
    ACCELBYTE_COMMON_API Error(const Error& other);
    ACCELBYTE_COMMON_API Error(error_type type, const String& what);
    ACCELBYTE_COMMON_API virtual ~Error() = default;

    ACCELBYTE_COMMON_API Error& operator=(const Error& other);

    ACCELBYTE_COMMON_API error_type type() const;
    ACCELBYTE_COMMON_API const String& what() const;

protected:
    error_type type_{ok};
    String what_{};
};

}