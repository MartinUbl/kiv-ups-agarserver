#include "General.h"
#include "Gameplay.h"
#include "Room.h"
#include "Log.h"

Gameplay::Gameplay() : m_lastRoomId(0)
{
    //
}

Gameplay::~Gameplay()
{
    //
}

void Gameplay::Init()
{
    // Create default room
    sLog->Info("Creating default room...");

    Room* rm = CreateRoom(GAME_TYPE_FREEFORALL, 30, "Default room", (uint32_t)MAP_DEFAULT_SIZE);
    if (rm)
    {
        // this will guarantee preserving even if the room is empty
        rm->SetAsDefault(true);
    }
}

void Gameplay::Shutdown()
{
    sLog->Info("Shutting down gameplay, destroying rooms...");

    std::unique_lock<std::recursive_mutex> lck(roomlist_mtx);

    for (std::map<uint32_t, Room*>::iterator itr = m_rooms.begin(); itr != m_rooms.end(); ++itr)
    {
        itr->second->SetRunning(false);
        itr->second->WaitForShutdown();
    }
}

uint32_t Gameplay::GenerateRoomId()
{
    // return next free room ID
    return ++m_lastRoomId;
}

Room* Gameplay::GetRoom(uint32_t id)
{
    std::unique_lock<std::recursive_mutex> lck(roomlist_mtx);

    // if it's not in rooms map, it does not exist
    if (m_rooms.find(id) == m_rooms.end())
        return nullptr;

    return m_rooms[id];
}

void Gameplay::DestroyRoom(uint32_t id)
{
    std::unique_lock<std::recursive_mutex> lck(roomlist_mtx);

    m_rooms.erase(id);
}

Room* Gameplay::CreateRoom(uint32_t gameType, uint32_t capacity, const char* name, uint32_t size)
{
    std::unique_lock<std::recursive_mutex> lck(roomlist_mtx);

    // create room record and put it into map
    Room* nroom = new Room(GenerateRoomId(), gameType, capacity, name, size);
    m_rooms[nroom->GetId()] = nroom;

    return nroom;
}

void Gameplay::GetRoomList(std::list<Room*> &target, int32_t gameType)
{
    std::unique_lock<std::recursive_mutex> lck(roomlist_mtx);

    // at first, clear target list to be filled
    target.clear();

    for (std::map<uint32_t, Room*>::iterator itr = m_rooms.begin(); itr != m_rooms.end(); ++itr)
    {
        // retrieve all rooms of specified type / any type if not specified
        if (gameType == GAME_TYPE_ANY || itr->second->GetGameType() == gameType)
            target.push_back(itr->second);
    }
}
