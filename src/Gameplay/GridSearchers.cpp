#include "General.h"
#include "WorldObject.h"
#include "Room.h"
#include "GridSearchers.h"
#include "Player.h"
#include "Network.h"
#include "Log.h"

void BaseCellVisitor::SetParameter(int32_t param)
{
    m_parameter = param;
}

void NearVisibilityGridSearcher::Execute()
{
    int32_t i, j;
    int32_t tmpX, tmpY;

    // iterate one left, center, one right
    for (i = -1; i <= 1; i++)
    {
        tmpX = (int32_t)m_cellX + i;
        // if we are out of grid range, continue to next
        if (tmpX < 0 || tmpX >= (int32_t)m_room->GetGridSizeX())
            continue;

        // iterate one up, center, one down
        for (j = -1; j <= 1; j++)
        {
            tmpY = (int32_t)m_cellY + j;
            // out of grid range
            if (tmpY < 0 || tmpY >= (int32_t)m_room->GetGridSizeY())
                continue;

            m_cellVisitor.Visit(m_room->GetCell(tmpX, tmpY));
        }
    }
}

void NearObjectVisibilityGridSearcher::Execute()
{
    int32_t i, j;
    uint32_t cellX, cellY;
    int32_t tmpX, tmpY;
    Position const& pos = m_subject->GetPosition();

    Cell::GetCoordPairFor(pos.x, pos.y, cellX, cellY);

    // iterate one left, center, one right
    for (i = -1; i <= 1; i++)
    {
        tmpX = (int32_t)cellX + i;
        // if we are out of grid range, continue to next
        if (tmpX < 0 || tmpX >= (int32_t)m_room->GetGridSizeX())
            continue;

        // iterate one up, center, one down
        for (j = -1; j <= 1; j++)
        {
            tmpY = (int32_t)cellY + j;
            // out of grid range
            if (tmpY < 0 || tmpY >= (int32_t)m_room->GetGridSizeY())
                continue;

            m_cellVisitor.Visit(m_room->GetCell(tmpX, tmpY));
        }
    }
}

void VisibilityChangeGridSearcher::Execute()
{
    int32_t i, j;

    int32_t nrx1, nrx2, nry1, nry2, orx1, orx2, ory1, ory2;
    // store border values of all cell neighbors
    orx1 = (int32_t)m_oldCellX - 1;
    orx2 = orx1 + 2;
    ory1 = (int32_t)m_oldCellY - 1;
    ory2 = ory1 + 2;

    nrx1 = (int32_t)m_newCellX - 1;
    nrx2 = nrx1 + 2;
    nry1 = (int32_t)m_newCellY - 1;
    nry2 = nry1 + 2;

    // store grid size for future use
    int32_t gsx = m_room->GetGridSizeX() - 1, gsy = m_room->GetGridSizeY() - 1;

    // make sure no cell neighbor coord value exceeds map border values
    if (orx1 < 0) orx1 = 0;
    if (nrx1 < 0) nrx1 = 0;
    if (ory1 < 0) ory1 = 0;
    if (nry1 < 0) nry1 = 0;
    if (orx2 > gsx) orx2 = gsx;
    if (nrx2 > gsx) nrx2 = gsx;
    if (ory2 > gsy) ory2 = gsy;
    if (nry2 > gsy) nry2 = gsy;

    // set destruction mode
    m_cellVisitor.SetParameter(1);
    // destroy for all out of range objects
    for (i = orx1; i <= orx2; i++)
    {
        for (j = ory1; j <= ory2; j++)
        {
            if (i >= nrx1 && i <= nrx2 && j >= nry1 && j <= nry2)
                continue;

            m_cellVisitor.Visit(m_room->GetCell(i, j));
        }
    }

    // set creation mode
    m_cellVisitor.SetParameter(0);
    // create for newly discovered objects
    for (i = nrx1; i <= nrx2; i++)
    {
        for (j = nry1; j <= nry2; j++)
        {
            if (i >= orx1 && i <= orx2 && j >= ory1 && j <= ory2)
                continue;

            m_cellVisitor.Visit(m_room->GetCell(i, j));
        }
    }
}

void CellDiscoveryGridSearcher::Execute()
{
    int32_t i, j;

    int32_t nrx1, nrx2, nry1, nry2, orx1, orx2, ory1, ory2;
    // store border values of all cell neighbors
    orx1 = (int32_t)m_oldCellX - 1;
    orx2 = orx1 + 2;
    ory1 = (int32_t)m_oldCellY - 1;
    ory2 = ory1 + 2;

    nrx1 = (int32_t)m_newCellX - 1;
    nrx2 = nrx1 + 2;
    nry1 = (int32_t)m_newCellY - 1;
    nry2 = nry1 + 2;

    // store grid size for future use
    int32_t gsx = m_room->GetGridSizeX() - 1, gsy = m_room->GetGridSizeY() - 1;

    // make sure no cell neighbor coord value exceeds map border values
    if (orx1 < 0) orx1 = 0;
    if (nrx1 < 0) nrx1 = 0;
    if (ory1 < 0) ory1 = 0;
    if (nry1 < 0) nry1 = 0;
    if (orx2 > gsx) orx2 = gsx;
    if (nrx2 > gsx) nrx2 = gsx;
    if (ory2 > gsy) ory2 = gsy;
    if (nry2 > gsy) nry2 = gsy;

    // set creation mode
    m_cellVisitor.SetParameter(0);
    // destroy all out of range objects
    for (i = nrx1; i <= nrx2; i++)
    {
        for (j = nry1; j <= nry2; j++)
        {
            if (i >= orx1 && i <= orx2 && j >= ory1 && j <= ory2)
                continue;

            m_cellVisitor.Visit(m_room->GetCell(i, j));
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

void MultiplexBroadcastPacketCellVisitor::Visit(Cell* cell)
{
    GamePacket &tosend = (m_parameter == 0) ? m_srcPacket1 : m_srcPacket2;

    for (std::list<Player*>::iterator itr = cell->playerList.begin(); itr != cell->playerList.end(); ++itr)
        sNetwork->SendPacket((*itr)->GetSession(), tosend);
}
