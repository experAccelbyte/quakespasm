// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include <string>

#include "logger_exports.h"
#include "LogSeverity.h"

namespace accelbyte {
namespace logger {

class LogRecord {
public:
    ACCELBYTE_LOGGER_API LogRecord();

    ACCELBYTE_LOGGER_API LogRecord(LogSeverity severity, std::string tag, std::string message);

    ACCELBYTE_LOGGER_API void tag(char* tag);

    ACCELBYTE_LOGGER_API auto tag() const noexcept -> const std::string&;

    ACCELBYTE_LOGGER_API void message(char* message);

    ACCELBYTE_LOGGER_API auto message() const noexcept -> const std::string&;

    ACCELBYTE_LOGGER_API void severity(LogSeverity severity);

    ACCELBYTE_LOGGER_API auto severity() const noexcept -> LogSeverity;

private:
    LogSeverity severity_{LogSeverity::verbose};
    std::string tag_{};
    std::string message_{};
};

} // namespace logger
} // namespace accelbyte
