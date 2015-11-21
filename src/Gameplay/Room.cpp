#include "General.h"
#include "Room.h"
#include "Player.h"
#include "Opcodes.h"
#include "Entities.h"
#include "GridSearchers.h"

#include <math.h>
#include <random>

void Cell::GetCoordPairFor(float x, float y, uint32_t &cellX, uint32_t &cellY)
{
    cellX = (uint32_t)floor(x / CELL_SIZE_X);
    cellY = (uint32_t)floor(y / CELL_SIZE_Y);
}

Room::Room(uint32_t id, uint32_t gameType, uint32_t capacity)
{
    m_id = id;
    m_gameType = gameType;
    m_capacity = capacity;

    m_lastObjectId = 0;

    // for now, this wouldn't vary
    SetMapSize(MAP_DEFAULT_SIZE_X, MAP_DEFAULT_SIZE_Y);

    // this is default for now, dunno if it will be adjustable in future
    GenerateRandomContent();
}

Room::~Room()
{
    for (std::list<Player*>::iterator itr = m_playerList.begin(); itr != m_playerList.end(); ++itr)
        (*itr)->SetRoomId(0);
}

void Room::SetMapSize(float sizeX, float sizeY)
{
    unsigned int i, j;

    m_sizeX = sizeX;
    m_sizeY = sizeY;

    // this is destructive action for now - do not use on fly, just before room initialization

    if (m_cellMap.size() > 0)
    {
        for (i = 0; i < m_cellMap.size(); i++)
            for (j = 0; j < m_cellMap[i].size(); j++)
                delete m_cellMap[i][j];
    }

    // init new grid (or cell map, if you like)
    m_cellMap.resize((size_t)floor(m_sizeX / CELL_SIZE_X) + 1);
    for (i = 0; i < m_cellMap.size(); i++)
    {
        m_cellMap[i].resize((size_t)floor(m_sizeY / CELL_SIZE_Y) + 1);
        for (j = 0; j < m_cellMap[i].size(); j++)
            m_cellMap[i][j] = new Cell(i, j);
    }
}

void Room::BroadcastPacket(GamePacket& pkt)
{
    for (std::list<Player*>::iterator itr = m_playerList.begin(); itr != m_playerList.end(); ++itr)
        sNetwork->SendPacket((*itr)->GetSession(), pkt);
}

void Cell::BroadcastPacket(GamePacket& pkt)
{
    for (std::list<Player*>::iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
        sNetwork->SendPacket((*itr)->GetSession(), pkt);
}

void Room::BroadcastPacketToNearCells(GamePacket& pkt, uint32_t centerCellX, uint32_t centerCellY)
{
    NearVisibilityGridSearcher gs(this, BroadcastPacketCellVisitor(pkt), centerCellX, centerCellY);

    gs.Execute();
}

void Room::AddPlayer(Player* player)
{
    if (m_playerList.size() >= m_capacity)
        return;

    m_playerList.push_back(player);
    player->SetRoomId(m_id);
}

void Room::RemovePlayer(Player* player)
{
    for (std::list<Player*>::iterator itr = m_playerList.begin(); itr != m_playerList.end(); ++itr)
    {
        if ((*itr)->GetId() == player->GetId())
        {
            player->SetRoomId(0);
            m_playerList.erase(itr);
            break;
        }
    }
}

void Room::PlaceNewPlayer(Player* player)
{
    // TODO: invent some genious algorithm to determine empty spot on map

    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    std::default_random_engine re;

    Position npos(m_sizeX * rnd(re), m_sizeY * rnd(re));
    player->Relocate(npos, false);
}

void Room::AddWorldObject(WorldObject* wobj)
{
    if (m_objectSet.find(wobj) != m_objectSet.end())
        return;

    uint32_t cellX, cellY;
    Position const& pos = wobj->GetPosition();

    Cell::GetCoordPairFor(pos.x, pos.y, cellX, cellY);
    if (cellX > m_cellMap.size() || cellY > m_cellMap[0].size())
        return;

    m_objectSet.insert(wobj);

    // add to grid
    m_cellMap[cellX][cellY]->objectList.push_back(wobj);

    // broadcast to cell and its neighbors, that we have a new object
    if (!m_playerList.empty())
    {
        GamePacket createPacket(SP_NEW_OBJECT);
        wobj->BuildCreatePacketBlock(createPacket);
        BroadcastPacketToNearCells(createPacket, cellX, cellY);
    }
}

void Room::RemoveWorldObject(WorldObject* wobj)
{
    if (m_objectSet.find(wobj) == m_objectSet.end())
        return;

    m_objectSet.erase(wobj);

    // cleanup from grid, etc.
    _RemoveWorldObject(wobj);
}

void Room::_RemoveWorldObject(WorldObject* wobj)
{
    uint32_t cellX, cellY;
    Position const& pos = wobj->GetPosition();

    Cell::GetCoordPairFor(pos.x, pos.y, cellX, cellY);
    if (cellX > m_cellMap.size() || cellY > m_cellMap[0].size())
        return;

    m_cellMap[cellX][cellY]->objectList.remove(wobj);

    GamePacket remPacket(SP_DESTROY_OBJECT);
    remPacket.WriteUInt32(wobj->GetId());
    remPacket.WriteUInt8(0); // TODO: destroy reason
    BroadcastPacketToNearCells(remPacket, cellX, cellY);
}

void Room::RelocateWorldObject(WorldObject* wobj, Position &oldpos)
{
    // TODO: this should be done slightly better than removing and adding, but since the game
    //       is not designed to move static objects, this probably won't happen at all

    RemoveWorldObject(wobj);
    AddWorldObject(wobj);
}

size_t Room::GetGridSizeX()
{
    return m_cellMap.size();
}

size_t Room::GetGridSizeY()
{
    return m_cellMap.size() > 0 ? m_cellMap[0].size() : 0;
}

Cell* Room::GetCell(uint32_t x, uint32_t y)
{
    if (x > m_cellMap.size())
        return nullptr;

    if (y > m_cellMap[x].size())
        return nullptr;

    return m_cellMap[x][y];
}

void Room::BuildObjectCreateBlock(GamePacket& pkt, Player* plr)
{
    // store position, where we will write object count later
    uint16_t sizePos = pkt.GetWritePos();
    // for now, write dummy value - zero
    pkt.WriteUInt32(0);

    AllObjectCreateCellVisitor visitor(pkt);
    NearObjectVisibilityGridSearcher gs(this, visitor, plr);

    gs.Execute();

    // overwrite zero value with valid count
    pkt.WriteUInt32At(visitor.GetCounter(), sizePos);
}

void Room::BuildPlayerCreateBlock(GamePacket& pkt, Player* plr)
{
    // store position, where we will write object count later
    uint16_t sizePos = pkt.GetWritePos();
    // for now, write dummy value - zero
    pkt.WriteUInt32(0);

    AllPlayerCreateCellVisitor visitor(pkt);
    NearObjectVisibilityGridSearcher gs(this, visitor, plr);

    gs.Execute();

    // overwrite zero value with valid count
    pkt.WriteUInt32At(visitor.GetCounter(), sizePos);
}

uint32_t Room::GetId()
{
    return m_id;
}

uint32_t Room::GetGameType()
{
    return m_gameType;
}

uint32_t Room::GetPlayerCount()
{
    return m_playerList.size();
}

uint32_t Room::GetCapacity()
{
    return m_capacity;
}

void Room::ClearAllObjects()
{
    // call internal cleanup method, we will take care of erasing from object set manually later
    for (std::set<WorldObject*>::iterator itr = m_objectSet.begin(); itr != m_objectSet.end(); ++itr)
        _RemoveWorldObject(*itr);

    m_objectSet.clear();
}

template <class T>
T* Room::CreateRoomObject(float x, float y)
{
    if (!std::is_base_of<WorldObject, T>::value)
        return nullptr;

    T* obj = new T();

    obj->SetId(++m_lastObjectId);
    obj->SetRoomId(GetId());
    obj->Relocate(Position(x, y), false);
    AddWorldObject(obj);

    return obj;
}

void Room::GenerateRandomContent()
{
    size_t i, j, k;

    ClearAllObjects();

    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    std::default_random_engine re;

    for (i = 0; i < m_cellMap.size(); i++)
    {
        for (j = 0; j < m_cellMap[i].size(); j++)
        {
            // let's say we have 50 eatable food in one cell
            for (k = 0; k < 50; k++)
                CreateRoomObject<IdleFoodEntity>(i*CELL_SIZE_X + rnd(re)*CELL_SIZE_X, j*CELL_SIZE_Y + rnd(re)*CELL_SIZE_Y);

            // and 2 bonuses
            for (k = 0; k < 2; k++)
                CreateRoomObject<BonusFoodEntity>(i*CELL_SIZE_X + rnd(re)*CELL_SIZE_X, j*CELL_SIZE_Y + rnd(re)*CELL_SIZE_Y);

            // and 2 traps
            for (k = 0; k < 2; k++)
                CreateRoomObject<TrapEntity>(i*CELL_SIZE_X + rnd(re)*CELL_SIZE_X, j*CELL_SIZE_Y + rnd(re)*CELL_SIZE_Y);
        }
    }
}
