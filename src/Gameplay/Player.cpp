#include "General.h"
#include "Player.h"
#include "Session.h"

Player::Player() : WorldObject(), m_roomId(0)
{
    m_session = new Session(this);
}

Player::~Player()
{
    //
}

Session* Player::GetSession()
{
    return m_session;
}

void Player::SetId(uint32_t id)
{
    m_id = id;
}

uint32_t Player::GetId()
{
    return m_id;
}

void Player::SetRoomId(uint32_t roomId)
{
    m_roomId = roomId;
}

uint32_t Player::GetRoomId()
{
    return m_roomId;
}
