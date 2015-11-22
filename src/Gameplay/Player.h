#ifndef AGAR_PLAYER_H
#define AGAR_PLAYER_H

#include "Network.h"
#include "WorldObject.h"

/* Player starting size */
#define DEFAULT_INITIAL_PLAYER_SIZE 10
/* Player size calculation coefficient used for calculations and in client also for drawing */
#define PLAYER_SIZE_CALC_COEF 0.3f/60.0f
/* Minimum player size considered in calculations */
#define MIN_PLAYER_CALC_SIZE 40

/* At this point, player will gain only half of all points to size */
#define PLAYER_REDUCE_INCOME_SIZE 120

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

        /* Sets player moving state */
        void SetMoving(bool state);
        /* Retrieves player moving state */
        bool IsMoving();

        /* Sets angle of player movement */
        void SetMoveAngle(float val);
        /* Retrieves player move angle */
        float GetMoveAngle();

        /* Update player record */
        void Update(uint32_t diff);

        /* Modifies player size */
        void ModifySize(int32_t mod);
        /* Retrieves player size */
        uint32_t GetSize();

        /* Sets dead flag */
        void SetDead(bool state);
        /* Is player dead? */
        bool IsDead();

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
        /* flag for player being dead */
        bool m_dead;
};

#endif
