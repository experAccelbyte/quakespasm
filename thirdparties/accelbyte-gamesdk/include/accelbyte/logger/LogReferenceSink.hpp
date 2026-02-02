// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "LogSinkInterface.hpp"

namespace accelbyte {
namespace logger {

template<typename TSink, typename = LogSinkInterface::enable_if_log_sink_t<TSink>>
class LogReferenceSink {
public:
    explicit LogReferenceSink(TSink& sink)
        : sink_{sink}
    {
    }

    auto operator<<(const  char* message) -> LogReferenceSink&
    {
        sink_ << message;
        return *this;
    }

private:
    TSink& sink_;
};

} // namespace logger
} // namespace accelbyte
