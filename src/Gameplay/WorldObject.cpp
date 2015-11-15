#include "General.h"
#include "WorldObject.h"

WorldObject::WorldObject()
{
    //
}

Position const& WorldObject::GetPosition()
{
    return m_position;
}
