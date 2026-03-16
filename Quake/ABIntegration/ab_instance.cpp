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
    p2p_.SetTaskRunner(task_runner_);
    matchmaking_.SetTaskRunner(task_runner_);
    matchmaking_.SetP2P(p2p_);
}

void ABInstance::SetServerUrl   (const char* url)    { server_url_    = url    ? url    : ""; }
void ABInstance::SetClientId    (const char* id)     { client_id_     = id     ? id     : ""; }
void ABInstance::SetClientSecret(const char* secret) { client_secret_ = secret ? secret : ""; }

const char* ABInstance::GetServerUrl()    const { return server_url_.c_str(); }
const char* ABInstance::GetClientId()     const { return client_id_.c_str(); }
const char* ABInstance::GetClientSecret() const { return client_secret_.c_str(); }

void ABInstance::Update()
{
    ams_.Update();
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
AB_P2P&               ABInstance::GetP2P()               { return p2p_; }
const AB_P2P&         ABInstance::GetP2P()         const { return p2p_; }
AB_AMS&               ABInstance::GetAMS()               { return ams_; }
const AB_AMS&         ABInstance::GetAMS()         const { return ams_; }
AB_Matchmaking&       ABInstance::GetMatchmaking()       { return matchmaking_; }
const AB_Matchmaking& ABInstance::GetMatchmaking() const { return matchmaking_; }

accelbyte::memory::SharedPtr<accelbyte::user::User> ABInstance::GetCurrentUser() const
{
    return login_.GetCurrentUser();
}
