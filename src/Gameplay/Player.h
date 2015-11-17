#ifndef AGAR_PLAYER_H
#define AGAR_PLAYER_H

#include "Network.h"
#include "WorldObject.h"

class Session;

/* Player class - object associated with one connected client 1:1 */
class Player : public WorldObject
{
    public:
        Player();
        ~Player();

        /* Retrieves stored session */
        Session* GetSession();

        /* Sets room ID the player has joined */
        void SetRoomId(uint32_t roomId);
        /* Retrieves player room ID */
        uint32_t GetRoomId();

        /* Sets player ID */
        void SetId(uint32_t id);
        /* Gets player ID */
        uint32_t GetId();

    protected:
        //

    private:
        /* Player ID */
        uint32_t m_id;
        /* Stored session associated with network client */
        Session* m_session;
        /* Player room ID */
        uint32_t m_roomId;
};

#endif
