// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "logger_exports.h"
#include "LogRecord.h"
#include "LogSinkInterface.hpp"


namespace accelbyte {
namespace logger {

class LogHandler {
public:
    ACCELBYTE_LOGGER_API constexpr static const char* format_tag_name{"tag"};
    ACCELBYTE_LOGGER_API constexpr static const char* format_severity_name{"severity"};
    ACCELBYTE_LOGGER_API constexpr static const char* format_message_name{"message"};

    struct EmptySinksConstructTag {};

    ACCELBYTE_LOGGER_API LogHandler();

    ACCELBYTE_LOGGER_API LogHandler(EmptySinksConstructTag);

    ACCELBYTE_LOGGER_API explicit LogHandler(LogSinkInterface sink);

    ACCELBYTE_LOGGER_API void operator()(const LogRecord& record);

    ACCELBYTE_LOGGER_API void set_severity(LogSeverity severity);

    ACCELBYTE_LOGGER_API auto add_sink(LogSinkInterface sink) -> LogSinkInterface&;

    ACCELBYTE_LOGGER_API auto get_sink_ptr_at(std::size_t index) noexcept -> LogSinkInterface*;

    ACCELBYTE_LOGGER_API auto get_sink_ptr_at(std::size_t index) const noexcept -> const LogSinkInterface*;

    ACCELBYTE_LOGGER_API void line_format(std::string format);

    ACCELBYTE_LOGGER_API auto line_format() const noexcept -> const std::string&;

private:
    LogSeverity severity_{LogSeverity::verbose};

    std::string line_format_{};

    std::vector<LogSinkInterface> log_sinks_{};

    auto log_severity_string(LogSeverity severity) -> std::string;
};

} // namespace logger
} // namespace accelbyte
