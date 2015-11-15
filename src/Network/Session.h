#ifndef AGAR_SESSION_H
#define AGAR_SESSION_H

#include "GamePacket.h"
#include "Network.h"
#include "Player.h"

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
        void SetConnectionInfo(SOCK socket, sockaddr_in &addr);
        /* Retrieves socket descriptor */
        SOCK GetSocket();
        /* Retrieves socket info */
        sockaddr_in const& GetSockAddr();

    private:
        /* Player pointer */
        Player* m_player;
        /* Socket descriptor */
        SOCK m_socket;
        /* Socket info */
        sockaddr_in m_sockAddr;
};

#endif
