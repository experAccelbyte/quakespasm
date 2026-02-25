// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "LogHandler.h"
#include "LogRecord.h"

#include "accelbyte/logger/metafunction.h"
#include "fmt/format.h"

// STL
#include <string>
#include <type_traits>
#include <utility>

namespace accelbyte {
namespace logger {

template<typename, typename... TFormatArgs>
struct is_fmt_format_args : std::false_type {};

template<typename... TFormatArgs>
struct is_fmt_format_args<accelbyte::void_t<decltype(fmt::format(std::declval<TFormatArgs>()...))>, TFormatArgs...>
    : std::true_type {};

template<typename... TFormatArgs>
using enable_if_fmt_format_args_t = std::enable_if_t<is_fmt_format_args<void, TFormatArgs...>::value>;

template<typename THandler = LogHandler>
class Logger {
public:
    using handler_type = THandler;

    explicit Logger(std::string tag = "DefaultTag", THandler handler = THandler{})
        : enabled_{true}
        , handler_{std::move(handler)}
        , tag_{std::move(tag)}
    {
    }

    auto is_enabled() const noexcept -> bool { return enabled_; }

    void enabled(bool value) noexcept { enabled_ = value; }

    auto get_handler() noexcept -> handler_type& { return handler_; }

    auto get_handler() const noexcept -> const handler_type& { return handler_; }

    template<typename... TFormatArgs, typename = enable_if_fmt_format_args_t<TFormatArgs...>>
    void log(LogSeverity severity, TFormatArgs&&... args)
    {
        if (!is_enabled()) {
            return;
        }

        const auto message = fmt::format(std::forward<TFormatArgs>(args)...);
        const auto record = LogRecord{severity, tag_, message};
        handler_(record);
    }

    template<typename... TFormatArgs, typename = enable_if_fmt_format_args_t<TFormatArgs...>>
    void verbose(TFormatArgs&&... args)
    {
        log(LogSeverity::verbose, std::forward<TFormatArgs>(args)...);
    }

    template<typename... TFormatArgs, typename = enable_if_fmt_format_args_t<TFormatArgs...>>
    void debug(TFormatArgs&&... args)
    {
        log(LogSeverity::debug, std::forward<TFormatArgs>(args)...);
    }

    template<typename... TFormatArgs, typename = enable_if_fmt_format_args_t<TFormatArgs...>>
    void info(TFormatArgs&&... args)
    {
        log(LogSeverity::info, std::forward<TFormatArgs>(args)...);
    }

    template<typename... TFormatArgs, typename = enable_if_fmt_format_args_t<TFormatArgs...>>
    void warning(TFormatArgs&&... args)
    {
        log(LogSeverity::warning, std::forward<TFormatArgs>(args)...);
    }

    template<typename... TFormatArgs, typename = enable_if_fmt_format_args_t<TFormatArgs...>>
    void error(TFormatArgs&&... args)
    {
        log(LogSeverity::error, std::forward<TFormatArgs>(args)...);
    }

private:
    bool enabled_{false};
    handler_type handler_{};
    std::string tag_{};
};

} // namespace logger
} // namespace accelbyte