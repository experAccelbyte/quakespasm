#include "ab_leaderboard.h"

#include <accelbyte/leaderboard/LeaderboardDataV3.h>
#include <accelbyte/leaderboard/leaderboard_data_v3/GetAllTimeRankV3.h>
#include <accelbyte/leaderboard/leaderboard_data_v3/GetCycleRankV3.h>
#include <accelbyte/leaderboard/leaderboard_data_v3/GetUserRankV3.h>
#include <accelbyte/leaderboard/models/Rank.h>
#include <accelbyte/leaderboard/models/UserRankV3.h>

extern "C" {
#include "quakedef.h"
#include "console.h"
}

#include <algorithm>
#include <chrono>
#include <cstring>
#include <future>

void AB_Leaderboard::SetTaskRunner(ABTaskRunner& tr)
{
    task_runner_ = &tr;
}

void AB_Leaderboard::PruneFutures()
{
    futures_.erase(
        std::remove_if(futures_.begin(), futures_.end(),
            [](const std::future<void>& f) {
                return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            }),
        futures_.end());
}

void AB_Leaderboard::FetchRankings(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* leaderboard_code, int limit, int offset)
{
    if (!user) {
        if (task_runner_)
            task_runner_->queue_task([]() {
                Con_Printf("AccelByte: FetchRankings called but user is null (not logged in)\n");
            });
        return;
    }

    PruneFutures();

    std::string lb_copy = leaderboard_code;

    futures_.push_back(std::async(std::launch::async,
        [this, user, lb_copy, limit, offset]()
    {
        accelbyte::leaderboard::leaderboard_data_v3::GetAllTimeRankV3 request;
        request.leaderboard_code = lb_copy.c_str();
        if (limit  > 0) request.limit  = limit;
        if (offset > 0) request.offset = offset;

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::leaderboard::LeaderboardDataV3::get_all_time_rank_v3(
            auth,
            request,
            [this, lb_copy](const accelbyte::leaderboard::model::Rank& result) {
                std::lock_guard<std::mutex> lock(mutex_);
                auto& entries = rankings_[lb_copy];
                entries.clear();
                int pos = 1;
                for (const auto& up : result.data)
                    entries.push_back({up.user_id.c_str(), up.point, pos++});
                int count = (int)entries.size();
                if (task_runner_)
                    task_runner_->queue_task([lb_copy, count]() {
                        Con_Printf("AccelByte: Leaderboard '%s' fetched %d entries\n",
                            lb_copy.c_str(), count);
                    });
            },
            [this, lb_copy](const accelbyte::Error& error) {
                std::string msg = error.what().c_str();
                if (task_runner_)
                    task_runner_->queue_task([lb_copy, msg]() {
                        Con_Printf("AccelByte: Failed to fetch leaderboard '%s' - %s\n",
                            lb_copy.c_str(), msg.c_str());
                    });
            }
        );
    }));
}

void AB_Leaderboard::FetchCycleRankings(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* leaderboard_code, const char* cycle_id,
    int limit, int offset)
{
    if (!user) {
        if (task_runner_)
            task_runner_->queue_task([]() {
                Con_Printf("AccelByte: FetchCycleRankings called but user is null (not logged in)\n");
            });
        return;
    }

    PruneFutures();

    std::string lb_copy    = leaderboard_code;
    std::string cycle_copy = cycle_id;

    futures_.push_back(std::async(std::launch::async,
        [this, user, lb_copy, cycle_copy, limit, offset]()
    {
        accelbyte::leaderboard::leaderboard_data_v3::GetCycleRankV3 request;
        request.leaderboard_code = lb_copy.c_str();
        request.cycle_id         = cycle_copy.c_str();
        if (limit  > 0) request.limit  = limit;
        if (offset > 0) request.offset = offset;

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::leaderboard::LeaderboardDataV3::get_cycle_rank_v3(
            auth,
            request,
            [this, lb_copy, cycle_copy](const accelbyte::leaderboard::model::Rank& result) {
                std::lock_guard<std::mutex> lock(mutex_);
                auto& entries = cycle_rankings_[lb_copy][cycle_copy];
                entries.clear();
                int pos = 1;
                for (const auto& up : result.data)
                    entries.push_back({up.user_id.c_str(), up.point, pos++});
                int count = (int)entries.size();
                if (task_runner_)
                    task_runner_->queue_task([lb_copy, cycle_copy, count]() {
                        Con_Printf("AccelByte: Cycle leaderboard '%s/%s' fetched %d entries\n",
                            lb_copy.c_str(), cycle_copy.c_str(), count);
                    });
            },
            [this, lb_copy, cycle_copy](const accelbyte::Error& error) {
                std::string msg = error.what().c_str();
                if (task_runner_)
                    task_runner_->queue_task([lb_copy, cycle_copy, msg]() {
                        Con_Printf("AccelByte: Failed to fetch cycle leaderboard '%s/%s' - %s\n",
                            lb_copy.c_str(), cycle_copy.c_str(), msg.c_str());
                    });
            }
        );
    }));
}

void AB_Leaderboard::FetchUserRank(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* leaderboard_code)
{
    if (!user) {
        if (task_runner_)
            task_runner_->queue_task([]() {
                Con_Printf("AccelByte: FetchUserRank called but user is null (not logged in)\n");
            });
        return;
    }

    PruneFutures();

    std::string lb_copy = leaderboard_code;
    std::string user_id = user->user_id().c_str();

    futures_.push_back(std::async(std::launch::async,
        [this, user, lb_copy, user_id]()
    {
        accelbyte::leaderboard::leaderboard_data_v3::GetUserRankV3 request;
        request.leaderboard_code = lb_copy.c_str();
        request.user_id          = user_id.c_str();

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::leaderboard::LeaderboardDataV3::get_user_rank_v3(
            auth,
            request,
            [this, lb_copy](const accelbyte::leaderboard::model::UserRankV3& result) {
                std::lock_guard<std::mutex> lock(mutex_);
                if (result.all_time.has_value()) {
                    const auto& at = result.all_time.value();
                    user_rankings_[lb_copy] = {at.rank, at.point};
                }
                auto& cycle_map = user_cycle_rankings_[lb_copy];
                cycle_map.clear();
                for (const auto& cr : result.cycles)
                    cycle_map[cr.cycle_id.c_str()] = {cr.rank, cr.point};
                if (task_runner_)
                    task_runner_->queue_task([lb_copy]() {
                        Con_Printf("AccelByte: User rank for leaderboard '%s' fetched\n",
                            lb_copy.c_str());
                    });
            },
            [this, lb_copy](const accelbyte::Error& error) {
                std::string msg = error.what().c_str();
                if (task_runner_)
                    task_runner_->queue_task([lb_copy, msg]() {
                        Con_Printf("AccelByte: Failed to fetch user rank for leaderboard '%s' - %s\n",
                            lb_copy.c_str(), msg.c_str());
                    });
            }
        );
    }));
}

int AB_Leaderboard::GetCachedRankings(
    const char* leaderboard_code, ab_rank_entry_t* out, int max_entries) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rankings_.find(leaderboard_code);
    if (it == rankings_.end())
        return 0;
    const auto& entries = it->second;
    int n = (int)std::min((size_t)max_entries, entries.size());
    for (int i = 0; i < n; ++i) {
        strncpy(out[i].user_id, entries[i].user_id.c_str(), 63);
        out[i].user_id[63] = '\0';
        out[i].point = entries[i].point;
        out[i].rank  = entries[i].rank;
    }
    return n;
}

int AB_Leaderboard::GetCachedCycleRankings(
    const char* leaderboard_code, const char* cycle_id,
    ab_rank_entry_t* out, int max_entries) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto outer = cycle_rankings_.find(leaderboard_code);
    if (outer == cycle_rankings_.end())
        return 0;
    auto inner = outer->second.find(cycle_id);
    if (inner == outer->second.end())
        return 0;
    const auto& entries = inner->second;
    int n = (int)std::min((size_t)max_entries, entries.size());
    for (int i = 0; i < n; ++i) {
        strncpy(out[i].user_id, entries[i].user_id.c_str(), 63);
        out[i].user_id[63] = '\0';
        out[i].point = entries[i].point;
        out[i].rank  = entries[i].rank;
    }
    return n;
}

bool AB_Leaderboard::GetCachedUserRank(
    const char* leaderboard_code, long* out_rank, float* out_point) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_rankings_.find(leaderboard_code);
    if (it == user_rankings_.end())
        return false;
    if (out_rank)  *out_rank  = it->second.rank;
    if (out_point) *out_point = it->second.point;
    return true;
}

bool AB_Leaderboard::GetCachedUserCycleRank(
    const char* leaderboard_code, const char* cycle_id,
    long* out_rank, float* out_point) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto outer = user_cycle_rankings_.find(leaderboard_code);
    if (outer == user_cycle_rankings_.end())
        return false;
    auto inner = outer->second.find(cycle_id);
    if (inner == outer->second.end())
        return false;
    if (out_rank)  *out_rank  = inner->second.rank;
    if (out_point) *out_point = inner->second.point;
    return true;
}

void AB_Leaderboard::InvalidateCache()
{
    std::lock_guard<std::mutex> lock(mutex_);
    rankings_.clear();
    cycle_rankings_.clear();
    user_rankings_.clear();
    user_cycle_rankings_.clear();
}
