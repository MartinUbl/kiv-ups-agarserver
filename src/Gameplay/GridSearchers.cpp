#include "General.h"
#include "WorldObject.h"
#include "Room.h"
#include "GridSearchers.h"
#include "Player.h"
#include "Network.h"

void NearVisibilityGridSearcher::Execute()
{
    int32_t i, j;
    uint32_t tmpX, tmpY;

    // iterate one left, center, one right
    for (i = -1; i <= 1; i++)
    {
        tmpX = m_cellX + i;
        // if we are out of grid range, continue to next
        if (tmpX < 0 || tmpX >= m_room->GetGridSizeX())
            continue;

        // iterate one up, center, one down
        for (j = -1; j <= 1; j++)
        {
            tmpY = m_cellY + j;
            // out of grid range
            if (tmpY < 0 || tmpY >= m_room->GetGridSizeY())
                continue;

            m_cellVisitor.Visit(m_room->GetCell(tmpX, tmpY));
        }
    }
}

void NearObjectVisibilityGridSearcher::Execute()
{
    int32_t i, j;
    uint32_t cellX, cellY, tmpX, tmpY;
    Position const& pos = m_subject->GetPosition();

    Cell::GetCoordPairFor(pos.x, pos.y, cellX, cellY);

    // iterate one left, center, one right
    for (i = -1; i <= 1; i++)
    {
        tmpX = cellX + i;
        // if we are out of grid range, continue to next
        if (tmpX < 0 || tmpX >= m_room->GetGridSizeX())
            continue;

        // iterate one up, center, one down
        for (j = -1; j <= 1; j++)
        {
            tmpY = cellY + j;
            // out of grid range
            if (tmpY < 0 || tmpY >= m_room->GetGridSizeY())
                continue;

            m_cellVisitor.Visit(m_room->GetCell(tmpX, tmpY));
        }
    }
}

void AllObjectCreateCellVisitor::Visit(Cell* cell)
{
    for (std::list<WorldObject*>::iterator itr = cell->objectList.begin(); itr != cell->objectList.end(); ++itr)
    {
        (*itr)->BuildCreatePacketBlock(m_targetPacket);
        m_counter++;
    }
}

uint32_t AllObjectCreateCellVisitor::GetCounter()
{
    return m_counter;
}

void AllPlayerCreateCellVisitor::Visit(Cell* cell)
{
    for (std::list<Player*>::iterator itr = cell->playerList.begin(); itr != cell->playerList.end(); ++itr)
    {
        (*itr)->BuildCreatePacketBlock(m_targetPacket);
        m_counter++;
    }
}

uint32_t AllPlayerCreateCellVisitor::GetCounter()
{
    return m_counter;
}

void BroadcastPacketCellVisitor::Visit(Cell* cell)
{
    for (std::list<Player*>::iterator itr = cell->playerList.begin(); itr != cell->playerList.end(); ++itr)
        sNetwork->SendPacket((*itr)->GetSession(), m_targetPacket);
}
