#include "General.h"
#include "Room.h"
#include "Player.h"

Room::Room(uint32_t id, uint32_t gameType, uint32_t capacity)
{
    m_id = id;
    m_gameType = gameType;
    m_capacity = capacity;
}

Room::~Room()
{
    for (std::list<Player*>::iterator itr = m_playerList.begin(); itr != m_playerList.end(); ++itr)
        (*itr)->SetRoomId(0);
}

void Room::BroadcastPacket(GamePacket& pkt)
{
    for (std::list<Player*>::iterator itr = m_playerList.begin(); itr != m_playerList.end(); ++itr)
        sNetwork->SendPacket((*itr)->GetSession(), pkt);
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
