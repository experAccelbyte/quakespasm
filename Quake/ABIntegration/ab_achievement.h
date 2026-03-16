#pragma once

#include "ab_task_runner.h"
#include "ab_integration.h"

// MSVC emits C2491 for `= default` constructors marked dllimport in AB SDK headers.
// This is a known SDK header quirk; suppress it for these includes only.
#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 2491)
#endif
#include <accelbyte/user/User.h>
#include <accelbyte/common/Error.h>
#include <accelbyte/memory/memory.h>
#ifdef _MSC_VER
#    pragma warning(pop)
#endif

#include <future>
#include <mutex>
#include <string>
#include <vector>

class AB_Achievement {
public:
    void SetTaskRunner(ABTaskRunner& tr);

    /*
     * Query all namespace achievements and the current user's unlock status.
     * Results are delivered asynchronously via on_done on the next AB_Update()
     * tick after the request completes.
     * The achievement list is cached after the first fetch; only user progress
     * is re-queried on subsequent calls.
     */
    void QueryAchievements(
        accelbyte::memory::SharedPtr<accelbyte::user::User> user,
        const char* user_id,
        ab_achievements_callback_t on_done);

    /*
     * Unlock a specific achievement for the logged-in user.
     * on_done is called on the main thread via AB_Update() when the
     * request completes (success or failure).
     */
    void UnlockAchievement(
        accelbyte::memory::SharedPtr<accelbyte::user::User> user,
        const char* user_id,
        const char* achievement_code,
        ab_unlock_achievement_callback_t on_done);

private:
    struct AchievementEntry {
        std::string achievement_code;
        std::string name;
        std::string description;
        bool        unlocked{false};
    };

    void PruneFutures();

    mutable std::mutex              mutex_;
    ABTaskRunner*                   task_runner_ = nullptr;
    std::vector<std::future<void>>  futures_;
    std::vector<AchievementEntry>   achievement_cache_;
    bool                            achievement_cache_valid_ = false;
};
