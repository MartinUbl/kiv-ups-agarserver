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
    m_color = 0x00000000;
    m_isMoving = false;
    m_moveAngle = 0.0f;
    m_dead = false;
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
    // 3.79 units per 500 ms
    const float msMove = 3.79f / 500.0f;

    if (IsMoving() && !IsDead())
    {
        float dx = cos(GetMoveAngle())*diff*msMove;
        float dy = sin(GetMoveAngle())*diff*msMove;

        Relocate(Position(m_position.x + dx, m_position.y + dy), true);

        Room* mroom = sGameplay->GetRoom(m_roomId);
        uint32_t cellX, cellY;
        Cell::GetCoordPairFor(m_position.x, m_position.y, cellX, cellY);
        Cell* mcell = mroom->GetCell(cellX, cellY);
        if (mcell)
        {
            WorldObject* closest = mroom->GetManhattanClosestObject(this);

            if (closest)
            {
                float closestDist = closest->GetPosition().DistanceExact(m_position);
                uint32_t compuSize = (m_playerSize > MIN_PLAYER_CALC_SIZE) ? m_playerSize : MIN_PLAYER_CALC_SIZE;
                uint32_t destSize = (closest->GetTypeId() == OBJECT_TYPE_PLAYER) ? ((Player*)closest)->GetSize() : 2;

                // we need to reach to the center with our border
                destSize /= 2;

                if (closestDist < (compuSize + destSize)*PLAYER_SIZE_CALC_COEF)
                {
                    // bigger player absorbs smaller player
                    if (closest->GetTypeId() == OBJECT_TYPE_PLAYER && destSize > m_playerSize)
                        mroom->EatObject((Player*)closest, this);
                    else
                        mroom->EatObject(this, closest);
                }
            }
        }
    }
}

void Player::ModifySize(int32_t mod)
{
    m_playerSize += mod;
}

uint32_t Player::GetSize()
{
    return m_playerSize;
}

void Player::SetSize(uint32_t size)
{
    m_playerSize = size;
}

void Player::SetDead(bool state)
{
    m_dead = state;
}

bool Player::IsDead()
{
    return m_dead;
}
