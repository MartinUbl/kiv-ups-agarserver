#ifndef AGAR_ROOM_H
#define AGAR_ROOM_H

#include "Network.h"

#include <set>
#include <functional>
#include <queue>

#define CELL_SIZE_X 10.0f
#define CELL_SIZE_Y 10.0f

/* 2 cells to left and 2 to right will be visible */
#define CELL_VISIBILITY_OFFSET 2

/* default map width */
#define MAP_DEFAULT_SIZE 500.0f

/* respawn time in seconds */
#define MAP_OBJECT_RESPAWN_TIME 60

/* number of seconds before room is shut down when empty */
#define ROOM_EMPTY_SHUTDOWN 60

class WorldObject;
struct Position;

struct Cell
{
    Cell(uint32_t x, uint32_t y) : coordX(x), coordY(y) { };

    uint32_t coordX, coordY;
    std::list<WorldObject*> objectList;
    std::list<Player*> playerList;

    static void GetCoordPairFor(float x, float y, uint32_t &cellX, uint32_t &cellY);

    void BroadcastPacket(GamePacket& pkt);
};

typedef std::vector<Cell*> CellColumn;
typedef std::vector<CellColumn> CellMap;

struct RespawnTimeComparator
{
    bool operator()(WorldObject* a, WorldObject* b);
};

/* Class holding information about one game room */
class Room
{
    public:
        /* Only one constructor - all parameters are mandatory */
        Room(uint32_t id, uint32_t gameType, uint32_t capacity, const char* name = "Unnamed room", uint32_t size = (uint32_t)MAP_DEFAULT_SIZE);
        ~Room();

        /* Adds player into room */
        void AddPlayer(Player* player);
        /* Removes player from room */
        void RemovePlayer(Player* player);
        /* Removes player from room */
        void RemovePlayerFromGrid(Player* player);
        /* Finds "empty" spot for new player and places him there */
        void PlaceNewPlayer(Player* player);
        /* Broadcasts packet inside room */
        void BroadcastPacket(GamePacket& pkt);
        /* Broadcasts packet to cell and its neighbors */
        void BroadcastPacketToNearCells(GamePacket& pkt, uint32_t centerCellX, uint32_t centerCellY);
        /* Builds stats packet and broadcasts it to all in room */
        void BroadcastStats();

        /* Builds objects update for player */
        void BuildObjectCreateBlock(GamePacket& pkt, Player* plr);
        /* Builds players update for player */
        void BuildPlayerCreateBlock(GamePacket& pkt, Player* plr);
        /* Build statistics packet block */
        void BuildStatsBlock(GamePacket& pkt);

        /* Adds world object to evidence and to grid */
        void AddWorldObject(WorldObject* wobj);
        /* Removes world object from evidence and grid */
        void RemoveWorldObject(WorldObject* wobj);
        /* Relocates world object between cells if necessary */
        void RelocateWorldObject(WorldObject* wobj, Position &oldpos);
        /* Relocates player between cells if necessary */
        void RelocatePlayer(Player* wobj, Position &oldpos);

        /* Retrieves closest object using manhattan distance */
        WorldObject* GetManhattanClosestObject(WorldObject* source);
        /* Player eats object */
        void EatObject(Player* plr, WorldObject* obj);
        /* Queues object for respawn */
        void QueueWorldObjectForRespawn(WorldObject* obj, uint32_t respawnDelay = MAP_OBJECT_RESPAWN_TIME);
        /* Respawns world object on its place */
        void RespawnObject(WorldObject* wobj);

        /* Retrieves room ID */
        uint32_t GetId();
        /* Retrieves game type */
        uint32_t GetGameType();
        /* Retrieves player count */
        uint32_t GetPlayerCount();
        /* Retrieves, how many players fit inside */
        uint32_t GetCapacity();

        /* Generates random contents of map */
        void GenerateRandomContent();
        /* Clears all objects (except players) from room */
        void ClearAllObjects();

        /* Gets grid width */
        size_t GetGridSizeX();
        /* Gets grid height */
        size_t GetGridSizeY();
        /* Retrieves one cell from grid */
        Cell* GetCell(uint32_t x, uint32_t y);

        /* Retrieves map width */
        float GetMapSizeX();
        /* Retrieves map height */
        float GetMapSizeY();

        /* Sets room name */
        void SetRoomName(const char* name);
        /* Retrieves room name */
        const char* GetRoomName();

        /* Sets "default" state of this room */
        void SetAsDefault(bool state);
        /* Is room listed as "default" ? */
        bool IsDefault();

        /* Updates room contents */
        void Update(uint32_t diff);

        /* Thread runner */
        void Run();

        /* Set running state */
        void SetRunning(bool state);

        /* waits for thread shutdown */
        void WaitForShutdown();

        /* lock for cell map updates */
        std::recursive_mutex cellMapLock;

    protected:
        /* For now protected, due to unsupported size variability; sets room dimensions */
        void SetMapSize(float sizeX, float sizeY);

        /* Internal method for cleaning up object from grid */
        void _RemoveWorldObject(WorldObject* wobj);

        /* Template method for creating object for this room */
        template <class T>
        T* CreateRoomObject(float x, float y);

    private:
        /* Basic parameters */
        uint32_t m_id, m_gameType, m_capacity;
        /* List of all players */
        std::list<Player*> m_playerList;
        /* List of all non-player objects */
        std::set<WorldObject*> m_objectSet;

        /* Room name */
        std::string m_roomName;
        /* Is room default? (allow empty state) */
        bool m_isDefault;

        /* Last assigned object ID */
        uint32_t m_lastObjectId;

        /* Respawn queue for respawning world objects */
        std::priority_queue<WorldObject*, std::vector<WorldObject*>, RespawnTimeComparator> m_respawnQueue;

        /* Map dimensions */
        float m_sizeX, m_sizeY;

        /* Map grid - we will do updates using simple grid and visibility detection */
        CellMap m_cellMap;

        /* Is still running? */
        bool IsRunning();

        /* last update time */
        uint32_t m_lastUpdateTime;

        /* when room recognizes its empty state */
        uint32_t m_emptyStateTime;

        /* is room running? */
        bool m_isRunning;

        /* update thread instance */
        std::thread* m_updateThread;

        /* generic miutex */
        std::mutex generic_mtx;
};

#endif
