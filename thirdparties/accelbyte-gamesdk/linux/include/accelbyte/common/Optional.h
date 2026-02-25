// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/memory/memory.h"
#include <stdexcept>

namespace accelbyte {

template<typename T>
class Optional {
public:
    Optional() {}
    Optional(const Optional& other)
    {
        if (other.has_value()) {
            emplace(other.value());
        }
    }
    ~Optional() {}

    operator T() const { return value(); }

    Optional<T>& operator=(const Optional<T>& other)
    {
        if (&other != this) {
            if (other.has_value()) {
                if (!has_value()) {
                    emplace(other.value());
                }
                else {
                    *value_ = other.value();
                }
            }
            else {
                reset();
            }
        }
        return *this;
    }

    T& operator=(const T& other)
    {
        if (!has_value()) {
            emplace(other);
        }
        else {
            *value_ = other;
        }
        return value();
    }

    bool operator==(const T& other) const
    {
        if (!has_value()) {
            return false;
        }
        return other == value();
    }

    bool operator==(const Optional<T>& other) const
    {
        if (!has_value()) {
            return !other.has_value();
        }
        return other == value();
    }

    bool operator!=(const T& other) const
    {
        return !(*this == other);
    }

    bool operator!=(const Optional<T>& other) const 
    { 
        return !(*this == other);
    }

    bool has_value() const { return value_ != nullptr; }

    void reset()
    {
        if (has_value()) {
            auto* val = value_.release();
            delete val;
        }
    }

    T& emplace()
    {
        if (!has_value()) {
            value_ = memory::make_unique_ptr<T>();
        }
        return value();
    }

    T& emplace(const T& val)
    {
        if (!has_value()) {
            value_ = memory::make_unique_ptr<T>(val);
        }
        return value();
    }

    T& value()
    {
        if (!has_value()) {
            throw std::runtime_error("Try to access value of empty accelbyte::Optional");
        }
        return *value_;
    }

    const T& value() const
    {
        if (!has_value()) {
            throw std::runtime_error("Try to access value of empty accelbyte::Optional");
        }
        return *value_;
    }

    T& value_or(T& option)
    {
        if (has_value()) {
            return *value_;
        }
        return option;
    }

    const T& value_or(const T& option) const
    {
        if (has_value()) {
            return *value_;
        }
        return option;
    }

private:
    memory::UniquePtr<T> value_;
};

} // namespace accelbyte