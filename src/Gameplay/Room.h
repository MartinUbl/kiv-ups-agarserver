#ifndef AGAR_ROOM_H
#define AGAR_ROOM_H

#include "Network.h"

/* Class holding information about one game room */
class Room
{
    public:
        /* Only one constructor - all parameters are mandatory */
        Room(uint32_t id, uint32_t gameType, uint32_t capacity);
        ~Room();

        /* Adds player into room */
        void AddPlayer(Player* player);
        /* Removes player from room */
        void RemovePlayer(Player* player);
        /* Broadcasts packet inside room */
        void BroadcastPacket(GamePacket& pkt);

        /* Retrieves room ID */
        uint32_t GetId();
        /* Retrieves game type */
        uint32_t GetGameType();
        /* Retrieves player count */
        uint32_t GetPlayerCount();
        /* Retrieves, how many players fit inside */
        uint32_t GetCapacity();

    protected:
        //

    private:
        /* Basic parameters */
        uint32_t m_id, m_gameType, m_capacity;
        /* List of all players */
        std::list<Player*> m_playerList;
};

#endif
