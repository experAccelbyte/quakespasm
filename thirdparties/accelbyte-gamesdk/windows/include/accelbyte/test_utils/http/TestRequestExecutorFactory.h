// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/RequestExecutorFactory.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/test_utils/test_utils_exports.h"

namespace accelbyte {
namespace test_utils {

/**
 * @brief a test request executor factory
 **/
class TestRequestExecutorFactory : public accelbyte::http::RequestExecutorFactory {
public:
    TestRequestExecutorFactory() = delete;
    AB_CNL_TESTS_UTILS_EXPORT TestRequestExecutorFactory(const accelbyte::http::RawAnswer& answer, accelbyte::Error ec);
    AB_CNL_TESTS_UTILS_EXPORT ~TestRequestExecutorFactory() = default;

    /**
     * @brief create an executor
     **/
    AB_CNL_TESTS_UTILS_EXPORT virtual memory::SharedPtr<accelbyte::http::RequestExecutor>
    make_executor() const override;

private:
    accelbyte::http::RawAnswer answer_;
    accelbyte::Error ec_;
};

} // namespace test_utils
} // namespace accelbyte
