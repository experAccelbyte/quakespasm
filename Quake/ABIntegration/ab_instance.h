#pragma once

#include "ab_login.h"
#include "ab_statistic.h"
#include "ab_cycle.h"
#include "ab_leaderboard.h"
#include "ab_p2p.h"
#include "ab_ams.h"
#include "ab_matchmaking.h"
#include "ab_achievement.h"
#include "ab_task_runner.h"

#include <string>

class ABInstance {
public:
    ABInstance();
    ~ABInstance() = default;

    void SetServerUrl   (const char* url);
    void SetClientId    (const char* id);
    void SetClientSecret(const char* secret);

    const char* GetServerUrl()    const;
    const char* GetClientId()     const;
    const char* GetClientSecret() const;

    void Update();

    AB_Login&             GetLogin();
    const AB_Login&       GetLogin() const;

    AB_Statistic&         GetStatistic();
    const AB_Statistic&   GetStatistic() const;
    AB_Cycle&             GetCycle();
    const AB_Cycle&       GetCycle() const;
    AB_Leaderboard&       GetLeaderboard();
    const AB_Leaderboard& GetLeaderboard() const;
    AB_P2P&               GetP2P();
    const AB_P2P&         GetP2P() const;
    AB_AMS&               GetAMS();
    const AB_AMS&         GetAMS() const;
    AB_Matchmaking&       GetMatchmaking();
    const AB_Matchmaking& GetMatchmaking() const;
    AB_Achievement&       GetAchievement();
    const AB_Achievement& GetAchievement() const;

    accelbyte::memory::SharedPtr<accelbyte::user::User> GetCurrentUser() const;

private:
    ABTaskRunner   task_runner_;
    AB_Login       login_;

    AB_Statistic   statistic_;
    AB_Cycle       cycle_;
    AB_Leaderboard leaderboard_;
    AB_P2P         p2p_;
    AB_AMS         ams_;
    AB_Matchmaking matchmaking_;
    AB_Achievement achievement_;

    std::string server_url_;
    std::string client_id_;
    std::string client_secret_;
};
