#ifndef AGAR_PLAYER_H
#define AGAR_PLAYER_H

#include "Network.h"
#include "WorldObject.h"

#define DEFAULT_INITIAL_PLAYER_SIZE 10

class Session;

/* Player class - object associated with one connected client 1:1 */
class Player : public WorldObject
{
    public:
        Player();
        ~Player();

        /* Retrieves stored session */
        Session* GetSession();

        /* Overrides object create block building function for player class */
        void BuildCreatePacketBlock(GamePacket& gp) override;

        /* Sets player name */
        void SetName(const char* name);
        /* Retrieves player name */
        const char* GetName();

    protected:
        //

    private:
        /* Stored session associated with network client */
        Session* m_session;

        /* player name */
        std::string m_name;
        /* player entity size */
        uint32_t m_playerSize;
        /* player color in 0RGB format (highest byte is all zero) */
        uint32_t m_color;
        /* moving flag for player */
        bool m_isMoving;
        /* the angle player is moving */
        float m_moveAngle;
};

#endif
