#include "ab_statistic.h"

#include <accelbyte/social/UserStatistic.h>
#include <accelbyte/social/user_statistic/UpdateUserStatItemValueV2.h>
#include <accelbyte/social/models/UpdateStatItem.h>

extern "C" {
#include "quakedef.h"
#include "console.h"
}

#include <algorithm>
#include <chrono>
#include <future>

void AB_Statistic::SetTaskRunner(ABTaskRunner& tr)
{
    task_runner_ = &tr;
}

void AB_Statistic::PruneFutures()
{
    futures_.erase(
        std::remove_if(futures_.begin(), futures_.end(),
            [](const std::future<void>& f) {
                return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            }),
        futures_.end());
}

void AB_Statistic::UpdateStat(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* stat_code, float value, int strategy)
{
    if (!user) {
        if (task_runner_)
            task_runner_->queue_task([]() {
                Con_Printf("AccelByte: UpdateStat called but user is null (not logged in)\n");
            });
        return;
    }

    using UpdateStrategy = accelbyte::social::model::UpdateStatItem::UpdateStrategy;
    UpdateStrategy update_strategy;
    switch (strategy)
    {
    case 0:  update_strategy = UpdateStrategy::OVERRIDE;   break;
    case 1:  update_strategy = UpdateStrategy::INCREMENT;  break;
    case 2:  update_strategy = UpdateStrategy::MAX;        break;
    case 3:  update_strategy = UpdateStrategy::MIN;        break;
    default:
        if (task_runner_)
            task_runner_->queue_task([strategy]() {
                Con_Printf("AccelByte: UpdateStat invalid strategy %d"
                    " (0=OVERRIDE 1=INCREMENT 2=MAX 3=MIN)\n", strategy);
            });
        return;
    }

    PruneFutures();

    std::string stat_copy = stat_code;
    std::string user_id   = user->user_id().c_str();

    futures_.push_back(std::async(std::launch::async,
        [this, user, stat_copy, user_id, value, update_strategy]()
    {
        accelbyte::social::user_statistic::UpdateUserStatItemValueV2 request;
        request.stat_code = stat_copy.c_str();
        request.user_id   = user_id.c_str();
        request.body.update_strategy = update_strategy;
        request.body.value = value;

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::social::UserStatistic::update_user_stat_item_value_v2(
            auth,
            request,
            [this, stat_copy](const accelbyte::social::model::StatItemInc& result) {
                float current = result.current_value;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    cache_[stat_copy] = current;
                }
                if (task_runner_)
                    task_runner_->queue_task([stat_copy, current]() {
                        Con_Printf("AccelByte: Stat '%s' updated, current value: %f\n",
                            stat_copy.c_str(), current);
                    });
            },
            [this, stat_copy](const accelbyte::Error& error) {
                std::string msg = error.what().c_str();
                if (task_runner_)
                    task_runner_->queue_task([stat_copy, msg]() {
                        Con_Printf("AccelByte: Failed to update stat '%s' - %s\n",
                            stat_copy.c_str(), msg.c_str());
                    });
            }
        );
    }));
}

void AB_Statistic::FetchStats(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* const* stat_codes, int count)
{
    if (!user) {
        if (task_runner_)
            task_runner_->queue_task([]() {
                Con_Printf("AccelByte: FetchStats called but user is null (not logged in)\n");
            });
        return;
    }

    // TODO: verify exact SDK header/symbol for list_user_stat_items
    if (task_runner_)
        task_runner_->queue_task([]() {
            Con_Printf("AccelByte: FetchStats — SDK call not yet implemented\n");
        });
    (void)stat_codes;
    (void)count;
}

void AB_Statistic::BulkUpdateStats(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* const* stat_codes, const float* values,
    int count, int strategy)
{
    if (!user) {
        if (task_runner_)
            task_runner_->queue_task([]() {
                Con_Printf("AccelByte: BulkUpdateStats called but user is null (not logged in)\n");
            });
        return;
    }

    // TODO: verify exact SDK header/symbol for bulk_update_user_stat_items_v2
    if (task_runner_)
        task_runner_->queue_task([]() {
            Con_Printf("AccelByte: BulkUpdateStats — SDK call not yet implemented\n");
        });
    (void)stat_codes;
    (void)values;
    (void)count;
    (void)strategy;
}

bool AB_Statistic::GetCachedValue(const char* stat_code, float* out_value) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(stat_code);
    if (it == cache_.end())
        return false;
    if (out_value)
        *out_value = it->second;
    return true;
}

void AB_Statistic::InvalidateCache()
{
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}
