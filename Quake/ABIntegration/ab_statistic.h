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

class AB_Statistic {
public:
    void SetTaskRunner(ABTaskRunner& tr);
    void UpdateStat     (accelbyte::memory::SharedPtr<accelbyte::user::User> user,
                         const char* stat_code, float value, int strategy);
    void FetchStats     (accelbyte::memory::SharedPtr<accelbyte::user::User> user,
                         const char* const* stat_codes, int count);
    void BulkUpdateStats(accelbyte::memory::SharedPtr<accelbyte::user::User> user,
                         const char* const* stat_codes, const float* values,
                         int count, int strategy);

    bool GetCachedValue  (const char* stat_code, float* out_value) const;
    void InvalidateCache ();

private:
    void PruneFutures();

    mutable std::mutex              mutex_;
    ABTaskRunner*                   task_runner_ = nullptr;
    std::vector<std::future<void>>  futures_;
    std::unordered_map<std::string, float>  cache_;
};
