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

    PruneFutures();

    std::string user_id = user->user_id().c_str();

    accelbyte::Vector<accelbyte::String> codes;
    for (int i = 0; i < count; i++)
        codes.push_back(stat_codes[i]);

    futures_.push_back(std::async(std::launch::async,
        [this, user, user_id, codes]()
    {
        accelbyte::social::user_statistic::GetUserStatItemsValueV2 request;
        request.user_id    = user_id.c_str();
        request.stat_codes = codes;

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::social::UserStatistic::get_user_stat_items_value_v2(
            auth,
            request,
            [this](const accelbyte::Vector<accelbyte::social::model::UserStatItemValue>& results) {
                int cached_count = 0;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    for (const auto& item : results) {
                        if (item.stat_code.has_value() && item.value.has_value()) {
                            cache_[item.stat_code.value().c_str()] = item.value.value();
                            cached_count++;
                        }
                    }
                }
                if (task_runner_)
                    task_runner_->queue_task([cached_count]() {
                        Con_Printf("AccelByte: Fetched and cached %d stat item(s)\n", cached_count);
                    });
            },
            [this](const accelbyte::Error& error) {
                std::string msg = error.what().c_str();
                if (task_runner_)
                    task_runner_->queue_task([msg]() {
                        Con_Printf("AccelByte: FetchStats failed - %s\n", msg.c_str());
                    });
            }
        );
    }));
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

    using UpdateStrategy = accelbyte::social::model::StatItemUpdates::UpdateStrategy;
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
                Con_Printf("AccelByte: BulkUpdateStats invalid strategy %d"
                    " (0=OVERRIDE 1=INCREMENT 2=MAX 3=MIN)\n", strategy);
            });
        return;
    }

    PruneFutures();

    std::string user_id = user->user_id().c_str();

    accelbyte::Vector<accelbyte::social::model::StatItemUpdates> items;
    for (int i = 0; i < count; i++) {
        accelbyte::social::model::StatItemUpdates item;
        item.stat_code       = stat_codes[i];
        item.update_strategy = update_strategy;
        item.value           = values[i];
        items.push_back(item);
    }

    futures_.push_back(std::async(std::launch::async,
        [this, user, user_id, items]()
    {
        accelbyte::social::user_statistic::UpdateUsersStatItemsValueV2 request;
        request.user_id = user_id.c_str();
        request.body    = items;

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::social::UserStatistic::update_users_stat_items_value_v2(
            auth,
            request,
            [this, item_count = items.size()](const accelbyte::Vector<accelbyte::social::model::StatOperations>& results) {
                int success_count = 0;
                int fail_count    = 0;
                for (const auto& op : results) {
                    if (op.success.has_value() && op.success.value())
                        success_count++;
                    else
                        fail_count++;
                }
                if (task_runner_)
                    task_runner_->queue_task([item_count, success_count, fail_count]() {
                        Con_Printf("AccelByte: BulkUpdateStats %zu item(s): %d succeeded, %d failed\n",
                            item_count, success_count, fail_count);
                    });
            },
            [this](const accelbyte::Error& error) {
                std::string msg = error.what().c_str();
                if (task_runner_)
                    task_runner_->queue_task([msg]() {
                        Con_Printf("AccelByte: BulkUpdateStats failed - %s\n", msg.c_str());
                    });
            }
        );
    }));
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
