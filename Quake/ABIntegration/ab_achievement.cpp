#include "ab_achievement.h"

extern "C" {
#include "quakedef.h"
#include "console.h"
}

// MSVC emits C2491 for `= default` constructors marked dllimport in AB SDK headers.
// This is a known SDK header quirk; suppress it for these includes only.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 2491)
#endif
#include <accelbyte/achievement/Achievements.h>
#include <accelbyte/achievement/UserAchievements.h>
#include <accelbyte/achievement/user_achievements/UnlockAchievement.h>
#include <accelbyte/tls/SecurityAuthorization.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <unordered_set>
#include <chrono>

void AB_Achievement::SetTaskRunner(ABTaskRunner& runner)
{
    task_runner_ = &runner;
}

void AB_Achievement::PruneFutures()
{
    futures_.erase(
        std::remove_if(
            futures_.begin(),
            futures_.end(),
            [](const std::future<void>& f) {
                return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            }),
        futures_.end());
}

void AB_Achievement::QueryAchievements(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* user_id,
    ab_achievements_callback_t on_done)
{
    if (!user || !task_runner_) {
        Con_Printf("AccelByte: Not logged in, cannot query achievements\n");
        if (on_done)
            on_done(nullptr, 0);
        return;
    }

    const bool cache_valid = achievement_cache_valid_;
    std::string uid = user_id ? user_id : "";

    std::lock_guard<std::mutex> lock(mutex_);
    PruneFutures();
    futures_.push_back(std::async(std::launch::async, [this, cache_valid, user, uid, on_done]() {
        // ----------------------------------------------------------------
        // Step 1: fetch all namespace achievements if cache is stale
        // ----------------------------------------------------------------
        if (!cache_valid) {
            std::promise<bool> prom;
            auto fut = prom.get_future();

            accelbyte::achievement::achievements::GetAchievements req;
            req.language = "en";
            req.limit = 100;

            const accelbyte::tls::SecurityAuthorization& auth = *user;

            accelbyte::achievement::Achievements::get_achievements(
                auth,
                req,
                [this, &prom](const accelbyte::achievement::model::Achievements& result) {
                    std::vector<AchievementEntry> entries;
                    entries.reserve(result.data.size());
                    for (const auto& a : result.data) {
                        AchievementEntry e;
                        e.achievement_code = a.achievement_code.c_str();
                        e.description = a.description.c_str();
                        e.name = a.name.c_str();
                        e.unlocked = false;
                        entries.push_back(std::move(e));
                    }
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        achievement_cache_ = std::move(entries);
                        achievement_cache_valid_ = true;
                    }
                    prom.set_value(true);
                },
                [&prom](const accelbyte::Error& error) {
                    Con_Printf("AccelByte: Failed to fetch achievements - %s\n", error.what().c_str());
                    prom.set_value(false);
                });

            if (!fut.get()) {
                task_runner_->queue_task([on_done]() {
                    if (on_done)
                        on_done(nullptr, 0);
                });
                return;
            }
        }

        // ----------------------------------------------------------------
        // Step 2: fetch user's unlocked achievements
        // ----------------------------------------------------------------
        {
            std::promise<bool> prom;
            auto fut = prom.get_future();

            accelbyte::achievement::user_achievements::GetUserAchievements req;
            req.user_id = uid.c_str();
            req.limit = 100;
            req.prefer_unlocked = true;

            const accelbyte::tls::SecurityAuthorization& auth = *user;

            accelbyte::achievement::UserAchievements::get_user_achievements(
                auth,
                req,
                [this, &prom](const accelbyte::achievement::model::UserAchievements& result) {
                    std::unordered_set<std::string> unlocked_codes;
                    for (const auto& ua : result.data) {
                        if (ua.status == 2) // 2 = unlocked per SDK docs
                            unlocked_codes.insert(ua.achievement_code.c_str());
                    }
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        for (auto& entry : achievement_cache_)
                            entry.unlocked = (unlocked_codes.count(entry.achievement_code) > 0);
                    }
                    prom.set_value(true);
                },
                [&prom](const accelbyte::Error& error) {
                    Con_Printf("AccelByte: Failed to fetch user achievements - %s\n", error.what().c_str());
                    prom.set_value(false);
                });

            if (!fut.get()) {
                task_runner_->queue_task([on_done]() {
                    if (on_done)
                        on_done(nullptr, 0);
                });
                return;
            }
        }

        // ----------------------------------------------------------------
        // Step 3: marshal cache to C structs and fire the delegate
        // ----------------------------------------------------------------
        std::vector<AchievementEntry> snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            snapshot = achievement_cache_;
        }

        task_runner_->queue_task([snapshot, on_done]() {
            if (!on_done)
                return;

            std::vector<ab_achievement_t> c_list;
            c_list.resize(snapshot.size());
            for (size_t i = 0; i < snapshot.size(); ++i) {
                ab_achievement_t& dst = c_list[i];
                strncpy(dst.achievement_code, snapshot[i].achievement_code.c_str(), sizeof(dst.achievement_code) - 1);
                dst.achievement_code[sizeof(dst.achievement_code) - 1] = '\0';
                strncpy(dst.name, snapshot[i].name.c_str(), sizeof(dst.name) - 1);
                dst.name[sizeof(dst.name) - 1] = '\0';
                strncpy(dst.description, snapshot[i].description.c_str(), sizeof(dst.description) - 1);
                dst.description[sizeof(dst.description) - 1] = '\0';
                dst.unlocked = snapshot[i].unlocked ? 1 : 0;
            }
            on_done(c_list.data(), static_cast<int>(c_list.size()));
        });
    }));
}

void AB_Achievement::UnlockAchievement(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const char* user_id,
    const char* achievement_code,
    ab_unlock_achievement_callback_t on_done)
{
    if (!user || !task_runner_) {
        Con_Printf("AccelByte: Not logged in, cannot unlock achievement\n");
        if (on_done)
            on_done(0, "Not logged in");
        return;
    }

    if (!achievement_code || !achievement_code[0]) {
        Con_Printf("AccelByte: achievement_code is empty\n");
        if (on_done)
            on_done(0, "achievement_code is empty");
        return;
    }

    std::string code(achievement_code);
    std::string uid = user_id ? user_id : "";

    // Skip if already unlocked in cache.
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& e : achievement_cache_) {
            if (e.achievement_code == code && e.unlocked) {
                Con_Printf("AccelByte: '%s' already unlocked\n", code.c_str());
                if (on_done)
                    on_done(1, nullptr);
                return;
            }
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    PruneFutures();
    futures_.push_back(std::async(std::launch::async, [this, user, uid, code, on_done]() {
        accelbyte::achievement::user_achievements::UnlockAchievement req;
        req.achievement_code = code.c_str();
        req.user_id = uid.c_str();

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::achievement::UserAchievements::unlock_achievement(
            auth,
            req,
            [this, code, on_done]() {
                Con_Printf("AccelByte: Achievement '%s' unlocked\n", code.c_str());
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    for (auto& e : achievement_cache_) {
                        if (e.achievement_code == code) {
                            e.unlocked = true;
                            break;
                        }
                    }
                }
                task_runner_->queue_task([on_done]() {
                    if (on_done)
                        on_done(1, nullptr);
                });
            },
            [this, code, on_done](const accelbyte::Error& error) {
                std::string msg = error.what().c_str();
                Con_Printf("AccelByte: Failed to unlock achievement '%s' - %s\n", code.c_str(), msg.c_str());
                task_runner_->queue_task([on_done, msg]() {
                    if (on_done)
                        on_done(0, msg.c_str());
                });
            });
    }));
}
