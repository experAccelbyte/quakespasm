// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/test_utils/test_utils_exports.h"
#include "accelbyte/memory/memory.h"

#include <functional>
#include "accelbyte/common/Vector.h"
#include <thread>

namespace accelbyte {
namespace test_utils {

class WebsocketEchoServer {
public:
    AB_CNL_TESTS_UTILS_EXPORT WebsocketEchoServer() = default;
    AB_CNL_TESTS_UTILS_EXPORT virtual ~WebsocketEchoServer(); 
    AB_CNL_TESTS_UTILS_EXPORT void start(uint16_t port);
    AB_CNL_TESTS_UTILS_EXPORT void stop();
    AB_CNL_TESTS_UTILS_EXPORT void onMessageRecievedCallback(const std::function<void(const Vector<char>&)>& callback);
    AB_CNL_TESTS_UTILS_EXPORT void onNewConnectionCallback(const std::function<void()>& callback);
    AB_CNL_TESTS_UTILS_EXPORT void onCloseConnectionCallback(const std::function<void()>& callback);

private:
    std::shared_ptr<std::thread> executor_{};
    std::function<void(const Vector<char>&)> messageCallback_;
    std::function<void()> newConnectionCallback_;
    std::function<void()> closeConnectionCallback_;
};

} // namespace test_utils
} // namespace accelbyte