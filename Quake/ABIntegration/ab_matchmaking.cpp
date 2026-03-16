/*
 * AccelByte Matchmaking Subsystem for QuakeSpasm
 */

#include "ab_matchmaking.h"
#include "ab_p2p.h"

// Lobby and WebSocket headers
#include <accelbyte/lobby/Lobby.h>
#include <accelbyte/lobby/LobbyConnection.h>
#include <accelbyte/lobby/TypedMessageHandler.h>
#include <accelbyte/lobby/notifications/OnMatchFound.h>
#include <accelbyte/lobby/notifications/OnGameSessionUpdated.h>
#include <accelbyte/lobby/notifications/OnDSStatusChanged.h>

// Session headers
#include <accelbyte/session/SessionService.h>
#include <accelbyte/session/GameSession.h>

// Match2 headers
#include <accelbyte/match2/Match2Service.h>
#include <accelbyte/match2/MatchTickets.h>

extern "C" {
#include "quakedef.h"
}

#include <string>

//------------------------------------------------------------------------------
// Cvars — registered in AB_Matchmaking::Init(), used in CreateMatchTicket /
// StartHosting.  File-local so they are not visible to other translation units.
//------------------------------------------------------------------------------
static cvar_t ab_match_pool = {"ab_match_pool", "quake_ffa", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_match_map  = {"ab_match_map",  "start",     CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};

//==============================================================================
// Handler classes
//==============================================================================

//------------------------------------------------------------------------------
// MatchFoundHandlerImpl — receives OnMatchFound from lobby WebSocket
//------------------------------------------------------------------------------
class MatchFoundHandlerImpl
    : public accelbyte::lobby::TypedMessageHandler<accelbyte::lobby::notifications::OnMatchFound>
{
    AB_Matchmaking* mm_;
public:
    explicit MatchFoundHandlerImpl(AB_Matchmaking* mm) : mm_(mm) {}

    void handle(const accelbyte::lobby::notifications::OnMatchFound& message) override
    {
        int num_players, num_teams;
        std::string match_id;
        ABTaskRunner* tr;
        AB_Matchmaking* mm = mm_;

        {
            std::lock_guard<std::mutex> lock(mm_->mutex_);

            mm_->status_          = AB_MM_FOUND;
            mm_->match_id_        = message.match_id.c_str();
            mm_->match_pool_name_ = message.match_pool.c_str();

            mm_->num_teams_   = (int)message.teams.size();
            mm_->num_players_ = 0;
            for (size_t i = 0; i < message.teams.size(); i++)
                mm_->num_players_ += (int)message.teams[i].size();

            num_players = mm_->num_players_;
            num_teams   = mm_->num_teams_;
            match_id    = mm_->match_id_;
            tr          = mm_->task_runner_;
        }

        if (tr)
            tr->queue_task([mm, match_id, num_players, num_teams](){
                Con_Printf("AccelByte: Match found! ID: %s, Players: %d, Teams: %d\n",
                    match_id.c_str(), num_players, num_teams);
                mm->JoinSession();
            });
    }
};

//------------------------------------------------------------------------------
// GameSessionUpdatedHandlerImpl — receives session updates (client path)
//------------------------------------------------------------------------------
class GameSessionUpdatedHandlerImpl
    : public accelbyte::lobby::TypedMessageHandler<accelbyte::lobby::notifications::OnGameSessionUpdated>
{
    AB_Matchmaking* mm_;
public:
    explicit GameSessionUpdatedHandlerImpl(AB_Matchmaking* mm) : mm_(mm) {}

    void handle(const accelbyte::lobby::notifications::OnGameSessionUpdated& message) override
    {
        std::string host_ip;
        ABTaskRunner* tr;

        {
            std::lock_guard<std::mutex> lock(mm_->mutex_);

            if (mm_->status_ != AB_MM_JOINED_AS_CLIENT)
                return;

            accelbyte::String ab_attrs = message.attributes;
            std::string attrs(ab_attrs.c_str());

            size_t pos = attrs.find("\"host_ip\"");
            if (pos != std::string::npos)
            {
                pos = attrs.find(':', pos);
                if (pos != std::string::npos)
                {
                    size_t start = attrs.find('"', pos + 1);
                    if (start != std::string::npos)
                    {
                        size_t end = attrs.find('"', start + 1);
                        if (end != std::string::npos)
                            host_ip = attrs.substr(start + 1, end - start - 1);
                    }
                }
            }

            if (!host_ip.empty())
            {
                mm_->host_address_ = host_ip;
                mm_->status_       = AB_MM_CONNECTING;
            }

            tr = mm_->task_runner_;
        }

        if (!host_ip.empty() && tr)
            tr->queue_task([host_ip](){
                Con_Printf("AccelByte: Host IP received: %s, connecting...\n", host_ip.c_str());
                Cbuf_AddText(va("connect \"%s\"\n", host_ip.c_str()));
                key_dest = key_game;
                m_state  = m_none;
            });
    }
};

//------------------------------------------------------------------------------
// DSStatusChangedHandlerImpl — receives DS status updates for DS sessions
//------------------------------------------------------------------------------
class DSStatusChangedHandlerImpl
    : public accelbyte::lobby::TypedMessageHandler<accelbyte::lobby::notifications::OnDSStatusChanged>
{
    AB_Matchmaking* mm_;
public:
    explicit DSStatusChangedHandlerImpl(AB_Matchmaking* mm) : mm_(mm) {}

    void handle(const accelbyte::lobby::notifications::OnDSStatusChanged& message) override
    {
        std::string session_id;
        accelbyte::memory::SharedPtr<accelbyte::user::User> user;
        ABTaskRunner* tr;
        AB_Matchmaking* mm = mm_;

        {
            std::lock_guard<std::mutex> lock(mm_->mutex_);

            if (mm_->status_ != AB_MM_WAITING_FOR_DS)
                return;

            session_id = mm_->session_id_;
            user       = mm_->current_user_;
            tr         = mm_->task_runner_;
        }

        if (!user)
            return;

        accelbyte::session::game_session::GetGameSession request;
        request.session_id = session_id.c_str();
        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::session::GameSession::get_game_session(
            auth, request,
            [mm, tr](const accelbyte::session::model::GameSession& session) {
                if (!mm->TryConnectFromDSInfo(session))
                {
                    if (tr)
                        tr->queue_task([](){
                            Con_Printf("AccelByte: DS status changed but not yet available, continuing to wait...\n");
                        });
                }
            },
            [tr](const accelbyte::Error& error) {
                std::string err = error.what().c_str();
                if (tr)
                    tr->queue_task([err](){
                        Con_Printf("AccelByte: Failed to fetch session: %s\n", err.c_str());
                    });
            }
        );
    }
};

//==============================================================================
// AB_Matchmaking implementation
//==============================================================================

AB_Matchmaking::AB_Matchmaking()  = default;
AB_Matchmaking::~AB_Matchmaking() = default;

void AB_Matchmaking::SetTaskRunner(ABTaskRunner& tr)
{
    task_runner_ = &tr;
}

void AB_Matchmaking::SetP2P(AB_P2P& p2p)
{
    p2p_ = &p2p;
}

void AB_Matchmaking::Init()
{
    static bool cvars_registered = false;
    if (!cvars_registered)
    {
        Cvar_RegisterVariable(&ab_match_pool);
        Cvar_RegisterVariable(&ab_match_map);
        cvars_registered = true;
    }
}

void AB_Matchmaking::Shutdown()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (lobby_connection_)
    {
        lobby_connection_->disconnect();
        lobby_connection_ = nullptr;
        Con_Printf("AccelByte: Lobby disconnected\n");
    }
    lobby_ = nullptr;

    match_found_handler_     = nullptr;
    session_updated_handler_ = nullptr;
    ds_status_handler_       = nullptr;

    status_ = AB_MM_IDLE;
    ticket_id_.clear();
    error_.clear();
    match_id_.clear();
    match_pool_name_.clear();
    num_players_ = 0;
    num_teams_   = 0;

    session_id_.clear();
    session_leader_id_.clear();
    is_session_leader_ = false;
    host_address_.clear();
    session_version_ = 0;
}

void AB_Matchmaking::Update()
{
    // Grab the connection under the mutex, then poll outside the lock.
    // (Message handlers acquire mutex_ themselves; holding it during read()
    // would deadlock.)
    accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> conn;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conn = lobby_connection_;
    }

    if (conn)
    {
        try { conn->read(); }
        catch (const std::exception& e) { (void)e; }
    }
}

//------------------------------------------------------------------------------
// SetupLobbyAfterLogin — called on the login background thread via AB_Login's
// post-login hook.  Connects the lobby WebSocket and registers handlers.
//------------------------------------------------------------------------------
void AB_Matchmaking::SetupLobbyAfterLogin(
    accelbyte::memory::SharedPtr<accelbyte::user::User> user,
    const std::string& user_id)
{
    try
    {
        auto lobby      = std::make_shared<accelbyte::lobby::Lobby>();
        auto connection = lobby->create_connection(*user);

        bool connected = connection->connect();

        {
            std::lock_guard<std::mutex> lock(mutex_);

            current_user_ = user;
            our_user_id_  = user_id;

            if (connected)
            {
                lobby_            = lobby;
                lobby_connection_ = connection;

                // Give the P2P subsystem the user and lobby connection it needs
                // for StartServer / ConnectToHost calls that come later.
                if (p2p_)
                    p2p_->SetLobbyContext(user, connection);

                match_found_handler_ = std::make_shared<MatchFoundHandlerImpl>(this);
                connection->add_message_handler(match_found_handler_);

                session_updated_handler_ = std::make_shared<GameSessionUpdatedHandlerImpl>(this);
                connection->add_message_handler(session_updated_handler_);

                ds_status_handler_ = std::make_shared<DSStatusChangedHandlerImpl>(this);
                connection->add_message_handler(ds_status_handler_);
            }
        }

        if (task_runner_)
        {
            if (connected)
                task_runner_->queue_task([](){
                    Con_Printf("AccelByte: Connected to lobby successfully\n");
                });
            else
                task_runner_->queue_task([](){
                    Con_Printf("AccelByte: Failed to connect to lobby\n");
                });
        }
    }
    catch (const std::exception& e)
    {
        std::string err = e.what();
        if (task_runner_)
            task_runner_->queue_task([err](){
                Con_Printf("AccelByte: Lobby connection error: %s\n", err.c_str());
            });
    }
}

//------------------------------------------------------------------------------
// TryConnectFromDSInfo — extract IP/port from DS info and issue connect
//------------------------------------------------------------------------------
bool AB_Matchmaking::TryConnectFromDSInfo(const accelbyte::session::model::GameSession& session)
{
    const auto& ds_info = session.ds_information;

    if (!ds_info.server.has_value())
        return false;

    const auto& server = ds_info.server.value();
    if (!server.ip.has_value() || !server.port.has_value())
        return false;

    std::string ip   = server.ip.value().c_str();
    int         port = server.port.value();
    std::string address = ip + ":" + std::to_string(port);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        host_address_ = address;
        status_       = AB_MM_CONNECTING;
    }

    if (task_runner_)
        task_runner_->queue_task([address](){
            Con_Printf("AccelByte: DS available at %s, connecting...\n", address.c_str());
            Cbuf_AddText(va("connect \"%s\"\n", address.c_str()));
            key_dest = key_game;
            m_state  = m_none;
        });

    return true;
}

//------------------------------------------------------------------------------
// PatchSessionWithHostIP — leader publishes their IP to the session attributes
//------------------------------------------------------------------------------
void AB_Matchmaking::PatchSessionWithHostIP()
{
    std::string session_id;
    int version;
    accelbyte::memory::SharedPtr<accelbyte::user::User> user;
    ABTaskRunner* tr;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        session_id = session_id_;
        version    = session_version_;
        user       = current_user_;
        tr         = task_runner_;
    }

    if (!user)
        return;

    std::string host_addr;
    if (my_tcpip_address[0])
        host_addr = std::string(my_tcpip_address) + ":" + std::to_string(net_hostport);
    else
        host_addr = "127.0.0.1:" + std::to_string(net_hostport);

    Con_Printf("AccelByte: Patching session with host IP: %s\n", host_addr.c_str());

    std::string attrs_json = "{\"host_ip\":\"" + host_addr + "\"}";

    future_ = std::async(std::launch::async, [user, session_id, version, attrs_json, tr](){
        accelbyte::session::game_session::PatchGameSession request;
        request.session_id      = session_id.c_str();
        request.body.attributes = accelbyte::utils::JsonObjectString(attrs_json.c_str());
        request.body.version    = version;

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::session::GameSession::patch_game_session(
            auth, request,
            [tr](const accelbyte::session::model::GameSession& /*session*/) {
                if (tr)
                    tr->queue_task([](){
                        Con_Printf("AccelByte: Session patched with host IP\n");
                    });
            },
            [tr](const accelbyte::Error& error) {
                std::string err = error.what().c_str();
                if (tr)
                    tr->queue_task([err](){
                        Con_Printf("AccelByte: Failed to patch session - %s\n", err.c_str());
                    });
            }
        );
    });
}

//------------------------------------------------------------------------------
// StartHosting — leader starts a Quake listen server
//------------------------------------------------------------------------------
void AB_Matchmaking::StartHosting()
{
    std::string map_name;
    int num_players;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        status_     = AB_MM_HOSTING;
        num_players = num_players_;
    }

    map_name = ab_match_map.string;
    if (map_name.empty())
        map_name = "start";

    Con_Printf("AccelByte: Starting host — map: %s, maxplayers: %d\n",
        map_name.c_str(), num_players);

    Cbuf_AddText("disconnect\n");
    Cbuf_AddText("listen 0\n");
    Cbuf_AddText(va("maxplayers %d\n", num_players));
    Cbuf_AddText(va("map %s\n", map_name.c_str()));

    key_dest = key_game;
    m_state  = m_none;

    if (p2p_)
        p2p_->StartServer();
}

//------------------------------------------------------------------------------
// JoinSession — join the AccelByte game session after match found
//------------------------------------------------------------------------------
void AB_Matchmaking::JoinSession()
{
    std::string match_id;
    accelbyte::memory::SharedPtr<accelbyte::user::User> user;
    std::string our_user_id;
    ABTaskRunner* tr;
    AB_Matchmaking* self = this;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (status_ != AB_MM_FOUND)
        {
            Con_Printf("AccelByte: Cannot join session — not in FOUND state\n");
            return;
        }

        match_id    = match_id_;
        status_     = AB_MM_JOINING;
        user        = current_user_;
        our_user_id = our_user_id_;
        tr          = task_runner_;
    }

    if (!user)
        return;

    Con_Printf("AccelByte: Joining session %s...\n", match_id.c_str());

    future_ = std::async(std::launch::async, [self, user, match_id, our_user_id, tr](){
        accelbyte::session::game_session::JoinGameSession request;
        request.session_id = match_id.c_str();

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::session::GameSession::join_game_session(
            auth, request,
            [self, our_user_id, tr](const accelbyte::session::model::GameSession& session) {
                std::string session_id   = session.id.c_str();
                std::string leader_id    = session.leader_id.c_str();
                int         version      = session.version;
                bool        is_leader    = (leader_id == our_user_id);
                std::string session_type = session.configuration.type.c_str();

                {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    self->session_id_        = session_id;
                    self->session_leader_id_ = leader_id;
                    self->session_version_   = version;
                    self->is_session_leader_ = is_leader;
                }

                if (session_type == "DS")
                {
                    if (self->TryConnectFromDSInfo(session))
                    {
                        if (tr)
                            tr->queue_task([session_id](){
                                Con_Printf("AccelByte: Joined session %s — DS available, connecting\n",
                                    session_id.c_str());
                            });
                    }
                    else
                    {
                        {
                            std::lock_guard<std::mutex> lock(self->mutex_);
                            self->status_ = AB_MM_WAITING_FOR_DS;
                        }
                        if (tr)
                            tr->queue_task([session_id](){
                                Con_Printf("AccelByte: Joined session %s — waiting for DS\n",
                                    session_id.c_str());
                            });
                    }
                }
                else
                {
                    {
                        std::lock_guard<std::mutex> lock(self->mutex_);
                        self->status_ = is_leader ? AB_MM_JOINED_AS_LEADER : AB_MM_JOINED_AS_CLIENT;
                    }

                    if (is_leader)
                    {
                        if (tr)
                            tr->queue_task([self, session_id](){
                                Con_Printf("AccelByte: Joined session %s as LEADER — starting host\n",
                                    session_id.c_str());
                                self->StartHosting();
                            });
                    }
                    else
                    {
                        if (tr)
                            tr->queue_task([self, session_id, leader_id](){
                                Con_Printf("AccelByte: Joined session %s as CLIENT — connecting to host via P2P (leader: %s)\n",
                                    session_id.c_str(), leader_id.c_str());
                                if (self->p2p_)
                                    self->p2p_->ConnectToHost(leader_id.c_str());
                            });
                    }
                }
            },
            [self, tr](const accelbyte::Error& error) {
                std::string err = error.what().c_str();
                {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    self->status_ = AB_MM_ERROR;
                    self->error_  = err;
                }
                if (tr)
                    tr->queue_task([err](){
                        Con_Printf("AccelByte: Failed to join session - %s\n", err.c_str());
                    });
            }
        );
    });
}

//------------------------------------------------------------------------------
// CreateMatchTicket — start searching for a match in the configured pool
//------------------------------------------------------------------------------
void AB_Matchmaking::CreateMatchTicket()
{
    const char* pool = ab_match_pool.string;
    if (!pool || !pool[0])
    {
        Con_Printf("AccelByte: ab_match_pool not configured\n");
        return;
    }

    accelbyte::memory::SharedPtr<accelbyte::user::User> user;
    std::string pool_copy;
    ABTaskRunner* tr;
    AB_Matchmaking* self = this;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!current_user_)
        {
            Con_Printf("AccelByte: Not logged in, cannot create match ticket\n");
            return;
        }

        user = current_user_;
        status_ = AB_MM_SEARCHING;
        ticket_id_.clear();
        error_.clear();
        match_id_.clear();
        match_pool_name_.clear();
        num_players_ = 0;
        num_teams_   = 0;
        pool_copy    = pool;
        tr           = task_runner_;
    }

    Con_Printf("AccelByte: Creating match ticket for pool '%s'...\n", pool);

    future_ = std::async(std::launch::async, [self, user, pool_copy, tr](){
        accelbyte::match2::match_tickets::CreateMatchTicket request;
        request.body.match_pool = pool_copy.c_str();

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::match2::MatchTickets::create_match_ticket(
            auth, request,
            [self, tr](const accelbyte::match2::model::MatchTicket& ticket) {
                std::string ticket_id = ticket.match_ticket_id.c_str();
                {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    self->ticket_id_ = ticket_id;
                }
                if (tr)
                    tr->queue_task([ticket_id](){
                        Con_Printf("AccelByte: Match ticket created: %s\n", ticket_id.c_str());
                    });
            },
            [self, tr](const accelbyte::Error& error) {
                std::string err = error.what().c_str();
                {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    self->status_ = AB_MM_ERROR;
                    self->error_  = err;
                }
                if (tr)
                    tr->queue_task([err](){
                        Con_Printf("AccelByte: Failed to create match ticket - %s\n", err.c_str());
                    });
            }
        );
    });
}

//------------------------------------------------------------------------------
// CancelMatchTicket — cancel a pending match ticket
//------------------------------------------------------------------------------
void AB_Matchmaking::CancelMatchTicket()
{
    std::string ticket_id;
    accelbyte::memory::SharedPtr<accelbyte::user::User> user;
    ABTaskRunner* tr;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        ticket_id = ticket_id_;
        status_   = AB_MM_CANCELLED;
        user      = current_user_;
        tr        = task_runner_;
    }

    if (ticket_id.empty())
    {
        Con_Printf("AccelByte: No match ticket to cancel\n");
        return;
    }

    if (!user)
        return;

    Con_Printf("AccelByte: Cancelling match ticket %s...\n", ticket_id.c_str());

    future_ = std::async(std::launch::async, [user, ticket_id, tr](){
        accelbyte::match2::match_tickets::DeleteMatchTicket request;
        request.ticketid = ticket_id.c_str();

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::match2::MatchTickets::delete_match_ticket(
            auth, request,
            [ticket_id, tr]() {
                if (tr)
                    tr->queue_task([ticket_id](){
                        Con_Printf("AccelByte: Match ticket %s cancelled\n", ticket_id.c_str());
                    });
            },
            [tr](const accelbyte::Error& error) {
                std::string err = error.what().c_str();
                if (tr)
                    tr->queue_task([err](){
                        Con_Printf("AccelByte: Failed to cancel match ticket - %s\n", err.c_str());
                    });
            }
        );
    });
}

//------------------------------------------------------------------------------
// Getters / Setters
//------------------------------------------------------------------------------

ab_matchmake_status_t AB_Matchmaking::GetStatus() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return status_;
}

void AB_Matchmaking::SetStatus(ab_matchmake_status_t s)
{
    std::lock_guard<std::mutex> lock(mutex_);
    status_ = s;
}

const char* AB_Matchmaking::GetErrorMessage() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return error_.empty() ? nullptr : error_.c_str();
}

const char* AB_Matchmaking::GetTicketId() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ticket_id_.empty() ? nullptr : ticket_id_.c_str();
}

const char* AB_Matchmaking::GetMatchId() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return match_id_.empty() ? nullptr : match_id_.c_str();
}

const char* AB_Matchmaking::GetMatchPoolName() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return match_pool_name_.empty() ? nullptr : match_pool_name_.c_str();
}

int AB_Matchmaking::GetNumPlayers() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return num_players_;
}

int AB_Matchmaking::GetNumTeams() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return num_teams_;
}

bool AB_Matchmaking::IsSessionLeader() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return is_session_leader_;
}
