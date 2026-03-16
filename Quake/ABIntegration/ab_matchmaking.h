/*
 * AccelByte Matchmaking Subsystem for QuakeSpasm
 *
 * Owns all lobby, session, and matchmaking state.
 * Wired into ABInstance via SetTaskRunner / SetP2P in the constructor.
 */

#pragma once

#include "ab_integration.h"   // ab_matchmake_status_t
#include "ab_task_runner.h"
#include <accelbyte/user/User.h>
#include <accelbyte/lobby/LobbyConnection.h>
#include <accelbyte/lobby/Lobby.h>
#include <accelbyte/session/models/GameSession.h>
#include <future>
#include <memory>
#include <mutex>
#include <string>

class AB_P2P;

// Handler types are defined in ab_matchmaking.cpp; forward-declared here so
// the destructor (also in .cpp) can destroy the shared_ptrs with full type info.
class MatchFoundHandlerImpl;
class GameSessionUpdatedHandlerImpl;
class DSStatusChangedHandlerImpl;

class AB_Matchmaking {
public:
    AB_Matchmaking();
    ~AB_Matchmaking();   // defined in .cpp — handler types are incomplete here

    AB_Matchmaking(const AB_Matchmaking&) = delete;
    AB_Matchmaking& operator=(const AB_Matchmaking&) = delete;

    void SetTaskRunner(ABTaskRunner& tr);
    void SetP2P(AB_P2P& p2p);

    void Init();       // register cvars (ab_match_pool, ab_match_map)
    void Shutdown();   // disconnect lobby, reset all state
    void Update();     // poll lobby WebSocket

    // Called from the post-login hook on the login background thread.
    // Connects the lobby WebSocket, registers notification handlers,
    // and stores current_user_ / our_user_id_ for later matchmaking calls.
    void SetupLobbyAfterLogin(
        accelbyte::memory::SharedPtr<accelbyte::user::User> user,
        const std::string& user_id);

    void CreateMatchTicket();
    void CancelMatchTicket();

    ab_matchmake_status_t GetStatus()        const;
    void                  SetStatus(ab_matchmake_status_t s);
    const char*           GetErrorMessage()  const;
    const char*           GetTicketId()      const;
    const char*           GetMatchId()       const;
    const char*           GetMatchPoolName() const;
    int                   GetNumPlayers()    const;
    int                   GetNumTeams()      const;
    bool                  IsSessionLeader()  const;

private:
    // Handler classes are defined in ab_matchmaking.cpp and need direct
    // member access for low-overhead callbacks.
    friend class MatchFoundHandlerImpl;
    friend class GameSessionUpdatedHandlerImpl;
    friend class DSStatusChangedHandlerImpl;

    bool TryConnectFromDSInfo(const accelbyte::session::model::GameSession& session);
    void PatchSessionWithHostIP();
    void StartHosting();
    void JoinSession();

    mutable std::mutex mutex_;

    // Set once after successful login, read by all matchmaking operations.
    accelbyte::memory::SharedPtr<accelbyte::user::User> current_user_;
    std::string our_user_id_;

    // Lobby
    std::shared_ptr<accelbyte::lobby::Lobby>                        lobby_;
    accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> lobby_connection_;

    // Handlers (owned here; complete types only in .cpp)
    std::shared_ptr<MatchFoundHandlerImpl>         match_found_handler_;
    std::shared_ptr<GameSessionUpdatedHandlerImpl> session_updated_handler_;
    std::shared_ptr<DSStatusChangedHandlerImpl>    ds_status_handler_;

    // Matchmaking state
    ab_matchmake_status_t status_{AB_MM_IDLE};
    std::string           ticket_id_;
    std::string           error_;
    std::string           match_id_;
    std::string           match_pool_name_;
    int                   num_players_{0};
    int                   num_teams_{0};

    // Session state
    std::string session_id_;
    std::string session_leader_id_;
    bool        is_session_leader_{false};
    std::string host_address_;
    int         session_version_{0};

    std::future<void> future_;

    ABTaskRunner* task_runner_{nullptr};
    AB_P2P*       p2p_{nullptr};
};
