#include "General.h"
#include "WorldObject.h"
#include "Gameplay.h"
#include "GamePacket.h"
#include "Room.h"

WorldObject::WorldObject()
{
    m_roomId = 0;
}

Position const& WorldObject::GetPosition()
{
    return m_position;
}

void WorldObject::SetTypeId(ObjectTypeId objType)
{
    m_typeId = objType;
}

void WorldObject::SetRoomId(uint32_t roomId)
{
    m_roomId = roomId;
}

uint32_t WorldObject::GetRoomId()
{
    return m_roomId;
}

void WorldObject::SetId(uint32_t id)
{
    m_id = id;
}

uint32_t WorldObject::GetId()
{
    return m_id;
}

void WorldObject::Relocate(Position &pos, bool update)
{
    Position oldPos(m_position);

    m_position.x = pos.x;
    m_position.y = pos.y;

    if (m_roomId && update)
    {
        Room* myRoom = sGameplay->GetRoom(m_roomId);
        if (myRoom)
            myRoom->RelocateWorldObject(this, oldPos);
    }
}

void WorldObject::BuildCreatePacketBlock(GamePacket& gp)
{
    gp.WriteUInt32(m_id);
    gp.WriteFloat(m_position.x);
    gp.WriteFloat(m_position.y);
    gp.WriteUInt8(m_typeId);

    gp.WriteUInt32(0); // TODO: specific parameter for each type
}
