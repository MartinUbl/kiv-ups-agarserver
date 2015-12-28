#ifndef AGAR_SESSION_H
#define AGAR_SESSION_H

#include "GamePacket.h"
#include "Network.h"
#include "Player.h"

#define MAX_SESSION_VIOLATIONS 3

/* Class holding information about session */
class Session
{
    public:
        /* Only constructor - Session must be bound to Player */
        Session(Player* plr);
        ~Session();

        /* Handles packet within session */
        void HandlePacket(GamePacket &packet);

        /* Retrieves Player pointer */
        Player* GetPlayer();

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
};

#endif
