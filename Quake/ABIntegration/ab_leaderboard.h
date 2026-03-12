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

class AB_Leaderboard {
public:
    void SetTaskRunner(ABTaskRunner& tr);
    // Fetch all-time global rankings. limit/offset <= 0 omits the optional fields.
    void FetchRankings(accelbyte::memory::SharedPtr<accelbyte::user::User> user,
                       const char* leaderboard_code, int limit, int offset);

    // Fetch cycle-scoped rankings for leaderboard_code + cycle_id.
    void FetchCycleRankings(accelbyte::memory::SharedPtr<accelbyte::user::User> user,
                            const char* leaderboard_code, const char* cycle_id,
                            int limit, int offset);

    // Fetch the current user's own rank (all-time + all associated cycles).
    void FetchUserRank(accelbyte::memory::SharedPtr<accelbyte::user::User> user,
                       const char* leaderboard_code);

    // Fill out[0..max_entries-1] with cached ranking data.
    // Returns the number of entries actually written (0 = not cached yet).
    int GetCachedRankings     (const char* leaderboard_code,
                               ab_rank_entry_t* out, int max_entries) const;
    int GetCachedCycleRankings(const char* leaderboard_code, const char* cycle_id,
                               ab_rank_entry_t* out, int max_entries) const;

    // Returns true + writes *out_rank/*out_point for the current user's all-time rank.
    bool GetCachedUserRank     (const char* leaderboard_code,
                                long* out_rank, float* out_point) const;
    bool GetCachedUserCycleRank(const char* leaderboard_code, const char* cycle_id,
                                long* out_rank, float* out_point) const;

    void InvalidateCache();

private:
    struct RankEntry     { std::string user_id; float point; int  rank; };
    struct UserRankEntry { long rank; float point; };

    void PruneFutures();

    mutable std::mutex              mutex_;
    ABTaskRunner*                   task_runner_ = nullptr;
    std::vector<std::future<void>>  futures_;

    // lb_code → entries (all-time)
    std::unordered_map<std::string, std::vector<RankEntry>> rankings_;

    // lb_code → cycle_id → entries
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::vector<RankEntry>>> cycle_rankings_;

    // lb_code → {rank, point}  (current user, all-time)
    std::unordered_map<std::string, UserRankEntry> user_rankings_;

    // lb_code → cycle_id → {rank, point}  (current user, per-cycle)
    std::unordered_map<std::string,
        std::unordered_map<std::string, UserRankEntry>> user_cycle_rankings_;
};
