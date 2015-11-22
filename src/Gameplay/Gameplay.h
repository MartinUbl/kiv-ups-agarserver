#ifndef AGAR_GAMEPLAY_H
#define AGAR_GAMEPLAY_H

#include "Singleton.h"

#include <map>
#include <list>

/* enumerator of known game types */
enum GameTypes
{
    GAME_TYPE_ANY = -1,
    GAME_TYPE_FREEFORALL = 0,
    GAME_TYPE_RATED = 1
};

class Room;

/* Gameplay class - contains all info needed for game - rooms management, etc. */
class Gameplay
{
    friend class Singleton<Gameplay>;
    public:
        ~Gameplay();

        /* Initializes gameplay - creates default rooms, etc. */
        void Init();
        /* Updates gameplay */
        void Update(uint32_t diff);

        /* Returns first free room ID */
        uint32_t GenerateRoomId();
        /* Retrieves room by its ID */
        Room* GetRoom(uint32_t id);

        /* Creates room using specified parameters */
        Room* CreateRoom(uint32_t gameType, uint32_t capacity);

        /* Fills supplied list with existing rooms of specified type */
        void GetRoomList(std::list<Room*> &target, int32_t gameType = GAME_TYPE_ANY);

    protected:
        /* Hidden singleton constructor */
        Gameplay();

    private:
        /* Last assigned room ID */
        uint32_t m_lastRoomId;
        /* Room map */
        std::map<uint32_t, Room*> m_rooms;
};

#define sGameplay Singleton<Gameplay>::getInstance()

#endif
