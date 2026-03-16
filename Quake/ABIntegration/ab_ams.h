/*
 * AMS Watchdog Protocol for QuakeSpasm Dedicated Server
 * Implements the AccelByte AMS watchdog WebSocket protocol so the DS
 * can signal readiness and receive drain notifications.
 */

#pragma once

#include "easywsclient.hpp"

#include <string>

class AB_AMS {
public:
    AB_AMS();
    ~AB_AMS();

    AB_AMS(const AB_AMS&) = delete;
    AB_AMS& operator=(const AB_AMS&) = delete;

    // Read DS_ID / WATCHDOG_URL and connect the watchdog WebSocket.
    // No-op if DS_ID is not present (client build or non-AMS server).
    void Init();

    // Close the watchdog WebSocket cleanly. Called by the destructor.
    void Shutdown();

    // Per-frame poll: send heartbeats, receive drain notifications.
    // Called from ABInstance::Update() every frame.
    void Update();

    // Signal AMS that the server is ready to accept players.
    void Ready();

    bool IsDraining() const;

private:
    void ProcessMessage(const std::string& msg);

    easywsclient::WebSocket::pointer ws_;
    std::string dsid_;
    std::string watchdog_url_;
    bool initialized_;
    bool draining_;
    bool ready_sent_;
    double last_heartbeat_time_;
};
