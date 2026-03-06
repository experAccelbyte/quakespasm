#pragma once

#include <accelbyte/user/User.h>
#include <accelbyte/common/Error.h>
#include "ab_integration.h"
#include "ab_task_runner.h"

#include <future>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class AB_Cycle {
public:
    void SetTaskRunner(ABTaskRunner& tr);
    // Fetch stat cycle items for the given cycle and optional stat code filter.
    // stat_codes/count may be 0/null to fetch all items in the cycle.
    void FetchCycleItems(accelbyte::memory::SharedPtr<accelbyte::user::User> user,
                         const char* cycle_id,
                         const char* const* stat_codes, int count);

    // Returns true and writes *out_value if a cached value exists for cycle_id+stat_code.
    bool GetCachedValue(const char* cycle_id, const char* stat_code,
                        float* out_value) const;

    // Clears the entire cache.
    void InvalidateCache();

private:
    void PruneFutures();

    // Outer key: cycle_id  Inner key: stat_code  Value: float
    mutable std::mutex              mutex_;
    ABTaskRunner*                   task_runner_ = nullptr;
    std::vector<std::future<void>>  futures_;
    std::unordered_map<std::string,
        std::unordered_map<std::string, float>>  cache_;
};
