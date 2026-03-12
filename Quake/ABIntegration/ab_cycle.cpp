#include "ab_cycle.h"

#include <accelbyte/social/UserStatisticCycle.h>
#include <accelbyte/social/user_statistic_cycle/GetUserStatCycleItems.h>
#include <accelbyte/social/models/UserStatCycleItems.h>

extern "C" {
#include "quakedef.h"
#include "console.h"
}

#include <algorithm>
#include <chrono>
#include <future>

void AB_Cycle::SetTaskRunner(ABTaskRunner& tr)
{
    task_runner_ = &tr;
}

void AB_Cycle::PruneFutures()
{
    futures_.erase(
        std::remove_if(futures_.begin(), futures_.end(),
            [](const std::future<void>& f) {
                return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            }),
        futures_.end());
}

void AB_Cycle::FetchCycleItems(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* cycle_id,
    const char* const* stat_codes, int count)
{
    if (!user) {
        if (task_runner_)
            task_runner_->queue_task([]() {
                Con_Printf("AccelByte: FetchCycleItems called but user is null (not logged in)\n");
            });
        return;
    }

    PruneFutures();

    std::string cycle_copy = cycle_id;
    std::string user_id    = user->user_id().c_str();

    std::string codes;
    for (int i = 0; i < count; ++i) {
        if (i > 0) codes += ',';
        codes += stat_codes[i];
    }

    futures_.push_back(std::async(std::launch::async,
        [this, user, cycle_copy, user_id, codes]()
    {
        accelbyte::social::user_statistic_cycle::GetUserStatCycleItems request;
        request.cycle_id = cycle_copy.c_str();
        request.user_id  = user_id.c_str();
        if (!codes.empty())
            request.stat_codes = accelbyte::String(codes.c_str());

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::social::UserStatisticCycle::get_user_stat_cycle_items(
            auth,
            request,
            [this, cycle_copy](const accelbyte::social::model::UserStatCycleItems& result) {
                int item_count = (int)result.data.size();
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    for (const auto& item : result.data)
                        cache_[cycle_copy][item.stat_code.c_str()] = item.value;
                }
                if (task_runner_)
                    task_runner_->queue_task([cycle_copy, item_count]() {
                        Con_Printf("AccelByte: Cycle '%s' fetched %d items\n",
                            cycle_copy.c_str(), item_count);
                    });
            },
            [this, cycle_copy](const accelbyte::Error& error) {
                std::string msg = error.what().c_str();
                if (task_runner_)
                    task_runner_->queue_task([cycle_copy, msg]() {
                        Con_Printf("AccelByte: Failed to fetch cycle '%s' - %s\n",
                            cycle_copy.c_str(), msg.c_str());
                    });
            }
        );
    }));
}

bool AB_Cycle::GetCachedValue(const char* cycle_id, const char* stat_code,
                               float* out_value) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto outer = cache_.find(cycle_id);
    if (outer == cache_.end())
        return false;
    auto inner = outer->second.find(stat_code);
    if (inner == outer->second.end())
        return false;
    if (out_value)
        *out_value = inner->second;
    return true;
}

void AB_Cycle::InvalidateCache()
{
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}
