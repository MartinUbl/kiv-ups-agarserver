#include "General.h"
#include "Player.h"
#include "Session.h"
#include "WorldObject.h"
#include "Gameplay.h"
#include "Room.h"
#include "Opcodes.h"

Player::Player() : WorldObject()
{
    SetTypeId(OBJECT_TYPE_PLAYER);
    m_session = new Session(this);

    m_playerSize = DEFAULT_INITIAL_PLAYER_SIZE;
    m_playerSpeed = MOVE_MS_COEF_MAX;
    m_color = 0x00000000;
    m_isMoving = false;
    m_moveAngle = 0.0f;
    m_dead = false;
    m_updateEnabled = false;
}

Player::~Player()
{
    //
}

Session* Player::GetSession()
{
    return m_session;
}

void Player::ResetAttributes()
{
    m_playerSize = DEFAULT_INITIAL_PLAYER_SIZE;
    m_isMoving = false;
    m_moveAngle = 0.0f;
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

void Player::Update(uint32_t diff)
{
    if (!IsUpdateEnabled())
        return;

    if (IsMoving() && !IsDead())
    {
        float dx = cos(GetMoveAngle())*diff*m_playerSpeed;
        float dy = sin(GetMoveAngle())*diff*m_playerSpeed;

        Position plpos(m_position.x + dx, m_position.y + dy);
        Relocate(plpos, true);
    }
}

void Player::ModifySize(int32_t mod)
{
    SetSize(m_playerSize + mod);
}

uint32_t Player::GetSize()
{
    return m_playerSize;
}

void Player::SetSize(uint32_t size)
{
    m_playerSize = size;

    if (m_playerSize <= 120)
        m_playerSpeed = MOVE_MS_COEF_MAX;
    else if (m_playerSize >= 1200)
        m_playerSpeed = MOVE_MS_COEF_MIN;
    else
        m_playerSpeed = -((int32_t)m_playerSize - 540)*(MOVE_MS_COEF_MIN / 420) + ((MOVE_MS_COEF_MAX + MOVE_MS_COEF_MIN) / 2.0f);
}

void Player::SetDead(bool state)
{
    m_dead = state;
}

bool Player::IsDead()
{
    return m_dead;
}

void Player::SetUpdateEnabled(bool state)
{
    m_updateEnabled = state;
}

bool Player::IsUpdateEnabled()
{
    return m_updateEnabled;
}
