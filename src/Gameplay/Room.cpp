#include "General.h"
#include "Room.h"
#include "Player.h"
#include "Opcodes.h"
#include "Entities.h"
#include "GridSearchers.h"
#include "Log.h"

#include <math.h>
#include <random>

/* Global position randomizer */
std::uniform_real_distribution<float> positionRandomizer(0.0f, 1.0f);
/* Global position randomizer engine */
std::default_random_engine positionRandomizerEngine;

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

float Room::GetMapSizeX()
{
    return m_sizeX;
}

float Room::GetMapSizeY()
{
    return m_sizeY;
}

void Room::Update(uint32_t diff)
{
    // update all players
    for (std::list<Player*>::iterator itr = m_playerList.begin(); itr != m_playerList.end(); ++itr)
        (*itr)->Update(diff);
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

void Room::RemovePlayerFromGrid(Player* player)
{
    uint32_t cellX, cellY;
    Position const& pos = player->GetPosition();

    Cell::GetCoordPairFor(pos.x, pos.y, cellX, cellY);
    // remove player from cellmap
    if (cellX < m_cellMap.size() && cellY < m_cellMap[0].size())
        m_cellMap[cellX][cellY]->playerList.remove(player);
}

void Room::RemovePlayer(Player* player)
{
    RemovePlayerFromGrid(player);

    // remove player from room
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

    Position npos(m_sizeX * positionRandomizer(positionRandomizerEngine), m_sizeY * positionRandomizer(positionRandomizerEngine));
    player->Relocate(npos, false);

    uint32_t cellX, cellY;

    Cell::GetCoordPairFor(npos.x, npos.y, cellX, cellY);

    // add to cell map
    m_cellMap[cellX][cellY]->playerList.push_back(player);

    if (!m_playerList.empty())
    {
        GamePacket createPacket(SP_NEW_PLAYER);
        player->BuildCreatePacketBlock(createPacket);
        BroadcastPacketToNearCells(createPacket, cellX, cellY);
    }
}

void Room::AddWorldObject(WorldObject* wobj)
{
    if (m_objectSet.find(wobj) != m_objectSet.end())
        return;

    uint32_t cellX, cellY;
    Position const& pos = wobj->GetPosition();

    Cell::GetCoordPairFor(pos.x, pos.y, cellX, cellY);
    if (cellX >= m_cellMap.size() || cellY >= m_cellMap[0].size())
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
    if (cellX >= m_cellMap.size() || cellY >= m_cellMap[0].size())
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

void Room::RelocatePlayer(Player* wobj, Position &oldpos)
{
    uint32_t cellX, cellY, cellXNew, cellYNew;
    uint16_t sizePos;
    Position const& pos = wobj->GetPosition();

    // retrieve old and new cell coords
    Cell::GetCoordPairFor(pos.x, pos.y, cellXNew, cellYNew);
    Cell::GetCoordPairFor(oldpos.x, oldpos.y, cellX, cellY);

    // if relocation between cells is needed, proceed
    if (cellX != cellXNew || cellY != cellYNew)
    {
        if (cellX >= m_cellMap.size() || cellY >= m_cellMap[0].size())
            return;
        if (cellXNew >= m_cellMap.size() || cellYNew >= m_cellMap[0].size())
            return;

        m_cellMap[cellX][cellY]->playerList.remove(wobj);
        m_cellMap[cellXNew][cellYNew]->playerList.push_back(wobj);

        // At first, let others know about moved player

        // prepare creation packet
        GamePacket createPacket(SP_NEW_PLAYER);
        wobj->BuildCreatePacketBlock(createPacket);

        // prepare destruction packet
        GamePacket deletePacket(SP_DESTROY_OBJECT);
        deletePacket.WriteUInt32(wobj->GetId());
        deletePacket.WriteUInt8(0);

        // create multiplexed broadcast packet visitor to send destroy packets to old area and create packets to new area
        MultiplexBroadcastPacketCellVisitor mpbc(createPacket, deletePacket);
        // this grid searcher will also check for overlappings, so when "new" and "old" area overlaps, no packet is sent to that area
        VisibilityChangeGridSearcher vsearch(this, mpbc, cellX, cellY, cellXNew, cellYNew);

        vsearch.Execute();

        // Next, let player know about newly discovered sorroundings

        GamePacket discoveryPacket(SP_UPDATE_WORLD);

        // store position, where we will write object count later
        sizePos = discoveryPacket.GetWritePos();
        // for now, write dummy value - zero
        discoveryPacket.WriteUInt32(0);

        AllPlayerCreateCellVisitor plrvisitor(discoveryPacket);
        CellDiscoveryGridSearcher plr_cdsearch(this, plrvisitor, cellX, cellY, cellXNew, cellYNew);

        plr_cdsearch.Execute();

        // overwrite zero value with valid count
        discoveryPacket.WriteUInt32At(plrvisitor.GetCounter(), sizePos);

        // store position, where we will write object count later
        sizePos = discoveryPacket.GetWritePos();
        // for now, write dummy value - zero
        discoveryPacket.WriteUInt32(0);

        AllObjectCreateCellVisitor objvisitor(discoveryPacket);
        CellDiscoveryGridSearcher obj_cdsearch(this, objvisitor, cellX, cellY, cellXNew, cellYNew);

        obj_cdsearch.Execute();

        // overwrite zero value with valid count
        discoveryPacket.WriteUInt32At(objvisitor.GetCounter(), sizePos);

        sNetwork->SendPacket(wobj->GetSession(), discoveryPacket);
    }

    // if the player is moving, send move heartbeat
    //if (wobj->IsMoving())
    // actually it does not have to move at all, this will broadcast position change to player sorroundings
    // regardless of move state
    {
        GamePacket heartbeat(SP_MOVE_HEARTBEAT);
        heartbeat.WriteUInt32(wobj->GetId());
        heartbeat.WriteFloat(pos.x);
        heartbeat.WriteFloat(pos.y);

        BroadcastPacketCellVisitor visitor(heartbeat);
        NearObjectVisibilityGridSearcher gs(this, visitor, wobj);

        gs.Execute();
    }
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
    if (x >= m_cellMap.size())
        return nullptr;

    if (y >= m_cellMap[x].size())
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

    for (i = 0; i < m_cellMap.size(); i++)
    {
        for (j = 0; j < m_cellMap[i].size(); j++)
        {
            // let's say we have 50 eatable food in one cell
            for (k = 0; k < 50; k++)
                CreateRoomObject<IdleFoodEntity>(i*CELL_SIZE_X + positionRandomizer(positionRandomizerEngine)*CELL_SIZE_X, j*CELL_SIZE_Y + positionRandomizer(positionRandomizerEngine)*CELL_SIZE_Y);

            // and 2 bonuses
            for (k = 0; k < 2; k++)
                CreateRoomObject<BonusFoodEntity>(i*CELL_SIZE_X + positionRandomizer(positionRandomizerEngine)*CELL_SIZE_X, j*CELL_SIZE_Y + positionRandomizer(positionRandomizerEngine)*CELL_SIZE_Y);

            // and 2 traps
            for (k = 0; k < 2; k++)
                CreateRoomObject<TrapEntity>(i*CELL_SIZE_X + positionRandomizer(positionRandomizerEngine)*CELL_SIZE_X, j*CELL_SIZE_Y + positionRandomizer(positionRandomizerEngine)*CELL_SIZE_Y);
        }
    }
}

void Room::EatObject(Player* plr, WorldObject* obj)
{
    GamePacket gp(SP_OBJECT_EATEN, 3 * 4);
    gp.WriteUInt32(obj->GetId());
    gp.WriteUInt32(plr->GetId());

    int32_t modSize = 0;

    if (obj->GetTypeId() == OBJECT_TYPE_IDLEFOOD)
        modSize = +2; // dummy value for now
    if (obj->GetTypeId() == OBJECT_TYPE_PLAYER)
    {
        gp.SetOpcode(SP_PLAYER_EATEN);
        modSize = (uint32_t)(((Player*)obj)->GetSize() * 0.66f);
    }

    // from certain point, player will gain only half of bonus to size
    if (plr->GetSize() >= PLAYER_REDUCE_INCOME_SIZE)
        modSize = modSize / 2;

    gp.WriteInt32(modSize);
    plr->ModifySize(modSize);

    BroadcastPacketCellVisitor visitor(gp);
    NearObjectVisibilityGridSearcher gs(this, visitor, obj);

    gs.Execute();

    if (obj->GetTypeId() == OBJECT_TYPE_PLAYER)
    {
        sLog->Info("Player %u ate player %u", plr->GetId(), obj->GetId());
        ((Player*)obj)->SetDead(true);
        RemovePlayerFromGrid((Player*)obj);
    }
    else
    {
        sLog->Info("Player %u ate object %u", plr->GetId(), obj->GetId());
        RemoveWorldObject(obj);
    }
}

WorldObject* Room::GetManhattanClosestObject(WorldObject* source)
{
    uint32_t sourceSize = 2;
    if (source->GetTypeId() == OBJECT_TYPE_PLAYER)
        sourceSize = ((Player*)source)->GetSize();

    ManhattanClosestCellVisitor visitor(source->GetPosition(), sourceSize, source);
    NearObjectVisibilityGridSearcher gs(this, visitor, source);

    gs.Execute();

    return visitor.GetFoundObject();
}
