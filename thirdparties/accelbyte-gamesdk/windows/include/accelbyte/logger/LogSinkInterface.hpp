// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/logger/metafunction.h"
#include "logger_exports.h"

// STL
#include <memory>
#include <type_traits>

namespace accelbyte {
namespace logger {

class LogSinkInterface {
    template<typename T, typename = void>
    struct is_log_sink : std::false_type {};

    template<typename T>
    struct is_log_sink<T, accelbyte::void_t<decltype(std::declval<T&>() << std::declval<const char*>())>>
        : std::true_type {};

public:
    template<typename T>
    using enable_if_log_sink_t = std::enable_if_t<is_log_sink<T>::value>;

    template<typename TModel, typename = enable_if_log_sink_t<TModel>>
    LogSinkInterface(TModel model)
        : concept_{std::make_unique<LogSinkModel<TModel>>(std::move(model))}
    {
    }

    ACCELBYTE_LOGGER_API LogSinkInterface(const LogSinkInterface& log_sink);

    ACCELBYTE_LOGGER_API LogSinkInterface(LogSinkInterface&& log_sink);

    ACCELBYTE_LOGGER_API auto operator=(const LogSinkInterface& log_sink)->LogSinkInterface&;

    ACCELBYTE_LOGGER_API auto operator=(LogSinkInterface&& log_sink)->LogSinkInterface&;

    ACCELBYTE_LOGGER_API auto operator<<(const char* message)->LogSinkInterface&;

private:
    struct LogSinkConcept {
        virtual ~LogSinkConcept() = default;

        virtual auto operator<<(const char* message) -> LogSinkConcept& = 0;

        virtual auto copy_() -> std::unique_ptr<LogSinkConcept> = 0;
    };

    template<typename TModel>
    struct LogSinkModel final : public LogSinkConcept {
        TModel model_;
        LogSinkModel(TModel model)
            : model_{std::move(model)}
        {
        }

        auto operator<<(const char* message) -> LogSinkConcept& override
        {
            model_ << message;
            return *this;
        }

        auto copy_() -> std::unique_ptr<LogSinkConcept> override
        {
            return std::make_unique<LogSinkModel<TModel>>(*this);
        }
    };

    std::unique_ptr<LogSinkConcept> concept_;
};

} // namespace logger
} // namespace accelbyte
