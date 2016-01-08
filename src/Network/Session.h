#ifndef AGAR_SESSION_H
#define AGAR_SESSION_H

#include "GamePacket.h"
#include "Network.h"

/* Maximum violations before disconnection */
#define MAX_SESSION_VIOLATIONS 3
/* Number of milliseconds between pings */
#define PING_TIMER 60000

/* Class holding information about session */
class Session
{
    public:
        /* Only constructor - Session must be bound to Player */
        Session(Player* plr);
        ~Session();

        /* Update session if needed */
        void Update(uint32_t diff);

        /* Handles packet within session */
        void HandlePacket(GamePacket &packet);

        /* Retrieves Player pointer */
        Player* GetPlayer();
        /* Overrides player pointer after i.e. session restore */
        void OverridePlayer(Player* pl);

        /* Sets connection info (socket descriptor and socket info) */
        void SetConnectionInfo(SOCK socket, sockaddr_in &addr, char* remoteAddr = nullptr);
        /* Retrieves socket descriptor */
        SOCK GetSocket();
        /* Retrieves socket info */
        sockaddr_in const& GetSockAddr();
        /* Retrieves remote address */
        const char* GetRemoteAddr();

        /* Sets connection state of associated client */
        void SetConnectionState(ConnectionState cstate);
        /* Retrueves connection state of associated client */
        ConnectionState GetConnectionState();

        /* Is session marked as expired? (should we disconnect client?) */
        bool IsMarkedAsExpired();
        /* Kick player and end session */
        void Kick();

        /* Creates, stores and returns generated session key */
        const char* CreateSessionKey();
        /* Retrieves session key */
        const char* GetSessionKey();

        /* Signals, that we received PING response */
        void SignalLatencyMeasure();
        /* Retrieves last measured latency */
        uint32_t GetLatency();

        /* Retrieves timeout value */
        time_t GetSessionTimeoutValue();
        /* Sets timeout value */
        void SetSessionTimeoutValue(time_t tm);

    protected:
        /* increases violation counter */
        void IncreaseViolationCounter();
        /* decreases violation counter */
        void DecreaseViolationCounter();
        /* clears violation counter */
        void ClearViolationCounter();

    private:
        /* Player pointer */
        Player* m_player;
        /* Socket descriptor */
        SOCK m_socket;
        /* Socket info */
        sockaddr_in m_sockAddr;
        /* Client connection state */
        ConnectionState m_connectionState;
        /* network violation counter */
        uint32_t m_violationCounter;
        /* is session expired? */
        bool m_isExpired;
        /* remote address */
        std::string m_remoteAddr;
        /* network latency */
        uint32_t m_latency;
        /* last ping send time */
        uint32_t m_lastPingSendTime;
        /* session key (for restoring session) */
        std::string m_sessionKey;

        /* time, when session times out */
        time_t m_sessionTimeout;
};

#endif
