#ifndef AGAR_ROOM_H
#define AGAR_ROOM_H

#include "Network.h"

#include <set>

#define CELL_SIZE_X 10.0f
#define CELL_SIZE_Y 10.0f

/* 2 cells to left and 2 to right will be visible */
#define CELL_VISIBILITY_OFFSET 2

#define MAP_DEFAULT_SIZE_X 500.0f
#define MAP_DEFAULT_SIZE_Y 500.0f

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
        /* Removes player from room */
        void RemovePlayerFromGrid(Player* player);
        /* Finds "empty" spot for new player and places him there */
        void PlaceNewPlayer(Player* player);
        /* Broadcasts packet inside room */
        void BroadcastPacket(GamePacket& pkt);
        /* Broadcasts packet to cell and its neighbors */
        void BroadcastPacketToNearCells(GamePacket& pkt, uint32_t centerCellX, uint32_t centerCellY);

        /* Builds objects update for player */
        void BuildObjectCreateBlock(GamePacket& pkt, Player* plr);
        /* Builds players update for player */
        void BuildPlayerCreateBlock(GamePacket& pkt, Player* plr);

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

        /* Updates room contents */
        void Update(uint32_t diff);

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

        /* Last assigned object ID */
        uint32_t m_lastObjectId;

        /* Map dimensions */
        float m_sizeX, m_sizeY;

        /* Map grid - we will do updates using simple grid and visibility detection */
        CellMap m_cellMap;
};

#endif
