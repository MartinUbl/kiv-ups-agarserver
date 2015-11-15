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

    protected:
        //

    private:
        /* Stored session associated with network client */
        Session* m_session;
};

#endif
