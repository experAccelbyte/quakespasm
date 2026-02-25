// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/common/Set.h"

#include <limits>

namespace accelbyte {
namespace utils {

template<typename T>
class IntegerPool {
public:
    IntegerPool()
        : IntegerPool(std::numeric_limits<T>::min(), std::numeric_limits<T>::max())
    {
    }
    IntegerPool(T min)
        : IntegerPool(min, std::numeric_limits<T>::max())
    {
    }
    IntegerPool(T min, T max)
        : current_(min)
        , min_(min)
        , max_(max)
    {
    }

    T get()
    {
        if (freed_.empty()) {
            T val = current_;
            current_++;
            return val;
        }
        auto begin = freed_.begin();
        T val = *begin;
        freed_.erase(begin);
        return val;
    }

    void free(T val) { freed_.insert(val); }

private:
    Set<T> freed_;
    T current_;
    T min_;
    T max_;
};

}
}