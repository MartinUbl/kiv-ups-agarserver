#ifndef AGAR_WORLDOBJECT_H
#define AGAR_WORLDOBJECT_H

#include <math.h>

/* Object type identifier */
enum ObjectTypeId
{
    OBJECT_TYPE_NONE = 0,
    OBJECT_TYPE_PLAYER = 1,
    OBJECT_TYPE_IDLEFOOD = 2,
    OBJECT_TYPE_BONUSFOOD = 3,
    OBJECT_TYPE_TRAP = 4,

    MAX_OBJECT_TYPE
};

#define PACKET_OBJECT_TYPE_PLAYER 0
#define PACKET_OBJECT_TYPE_WORLDOBJECT 1

/* Wrapper structure for position in plane */
struct Position
{
    /* X coordinate */
    float x;
    /* Y coordinate */
    float y;

    /* Default constructor nulling all coordinates */
    Position() : x(0.0f), y(0.0f) { };
    /* Constructor with initial coordinates specified */
    Position(float mx, float my) : x(mx), y(my) { };

    /* Computes exact distance between two points on plane */
    float DistanceExact(Position const& dst) const
    {
        return sqrt(powf(x-dst.x, 2) + powf(y-dst.y, 2));
    }

    /* Computed Manhattan distance between two points on plane */
    float DistanceManhattan(Position const& dst) const
    {
        return fabs(x - dst.x) + fabs(y - dst.y);
    }
};

class GamePacket;
class Player;

/* Class for identifying base object in world */
class WorldObject
{
    public:
        WorldObject();

        /* Retrieves object position */
        Position const& GetPosition();

        /* Relocates object */
        void Relocate(Position &pos, bool update = true);

        /* Sets room ID the player has joined */
        void SetRoomId(uint32_t roomId);
        /* Retrieves player room ID */
        uint32_t GetRoomId();

        /* Sets player ID */
        void SetId(uint32_t id);
        /* Gets player ID */
        uint32_t GetId();

        /* Retrieves object type ID */
        ObjectTypeId GetTypeId();

        /* Retrieves packet-oriented object type ID */
        uint8_t GetPacketTypeId();

        /* Sets, when the object should be respawned */
        void SetRespawnTime(time_t when);
        /* Retrieves respawn time */
        time_t GetRespawnTime();

        /* Builds create packet contents to be sent to players; this method assumes valid opcode has been set */
        virtual void BuildCreatePacketBlock(GamePacket& gp);

        /* When eaten by player */
        virtual void OnEatenByPlayer(Player* plr) { };

    protected:
        /* "Family" visible method for setting object type id */
        void SetTypeId(ObjectTypeId objType);

        /* Object position */
        Position m_position;

        /* Object type identifier */
        ObjectTypeId m_typeId;

        /* Object room ID */
        uint32_t m_roomId;

        /* Player ID */
        uint32_t m_id;

        /* Will respawn at .. */
        time_t m_respawnTime;

    private:
        //
};

#endif
