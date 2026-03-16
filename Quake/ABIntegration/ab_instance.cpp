#include "ab_instance.h"

extern "C" {
#include "quakedef.h"
#include "console.h"
}

ABInstance::ABInstance()
{
    Con_Printf("AccelByte: Instance created.\n");

    login_.SetTaskRunner(task_runner_);
    statistic_.SetTaskRunner(task_runner_);
    cycle_.SetTaskRunner(task_runner_);
    leaderboard_.SetTaskRunner(task_runner_);
    achievement_.SetTaskRunner(task_runner_);
}

void ABInstance::SetServerUrl   (const char* url)    { server_url_    = url    ? url    : ""; }
void ABInstance::SetClientId    (const char* id)     { client_id_     = id     ? id     : ""; }
void ABInstance::SetClientSecret(const char* secret) { client_secret_ = secret ? secret : ""; }

const char* ABInstance::GetServerUrl()    const { return server_url_.c_str(); }
const char* ABInstance::GetClientId()     const { return client_id_.c_str(); }
const char* ABInstance::GetClientSecret() const { return client_secret_.c_str(); }

void ABInstance::Update()
{
    task_runner_.execute_task_queue();
}

AB_Login&       ABInstance::GetLogin()       { return login_; }
const AB_Login& ABInstance::GetLogin() const { return login_; }

AB_Statistic&       ABInstance::GetStatistic()           { return statistic_; }
const AB_Statistic& ABInstance::GetStatistic() const     { return statistic_; }
AB_Cycle&           ABInstance::GetCycle()               { return cycle_; }
const AB_Cycle&     ABInstance::GetCycle() const         { return cycle_; }
AB_Leaderboard&       ABInstance::GetLeaderboard()       { return leaderboard_; }
const AB_Leaderboard& ABInstance::GetLeaderboard() const { return leaderboard_; }
AB_Achievement&       ABInstance::GetAchievement()       { return achievement_; }
const AB_Achievement& ABInstance::GetAchievement() const { return achievement_; }

accelbyte::memory::SharedPtr<accelbyte::user::User> ABInstance::GetCurrentUser() const
{
    return login_.GetCurrentUser();
}
