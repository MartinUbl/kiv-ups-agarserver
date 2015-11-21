#include "General.h"
#include "Player.h"
#include "Session.h"
#include "WorldObject.h"

Player::Player() : WorldObject()
{
    SetTypeId(OBJECT_TYPE_PLAYER);
    m_session = new Session(this);

    m_playerSize = DEFAULT_INITIAL_PLAYER_SIZE;
    m_color = 0x00000000;
    m_isMoving = false;
    m_moveAngle = 0.0f;
}

Player::~Player()
{
    //
}

Session* Player::GetSession()
{
    return m_session;
}

void Player::SetName(const char* name)
{
    m_name = std::string(name);
}

const char* Player::GetName()
{
    return m_name.c_str();
}

void Player::BuildCreatePacketBlock(GamePacket& gp)
{
    gp.WriteUInt32(m_id);
    gp.WriteString(m_name.c_str());
    gp.WriteUInt32(m_playerSize);
    gp.WriteFloat(m_position.x);
    gp.WriteFloat(m_position.y);
    gp.WriteUInt32(m_color);
    gp.WriteUInt8(m_isMoving ? 1 : 0);
    gp.WriteFloat(m_moveAngle);
}

void Player::SetMoving(bool state)
{
    m_isMoving = state;
}

bool Player::IsMoving()
{
    return m_isMoving;
}

void Player::SetMoveAngle(float val)
{
    m_moveAngle = val;
}

float Player::GetMoveAngle()
{
    return m_moveAngle;
}
