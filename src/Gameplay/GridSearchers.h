#ifndef AGAR_GRIDSEARCH_H
#define AGAR_GRIDSEARCH_H

#include "WorldObject.h"
#include "Room.h"

class GamePacket;

/*********************
 * Base class section
 *********************/

/* Base for all cell visitors */
class BaseCellVisitor
{
    public:
        BaseCellVisitor() { };

        /* Visits cell and performs derived action on it */
        virtual void Visit(Cell* cell) = 0;
};

/* Base for all grid searchers */
class BaseGridSearcher
{
    public:
        BaseGridSearcher(Room* room, BaseCellVisitor &visitor) : m_room(room), m_cellVisitor(visitor) { };

        /* Proceeds to search and visit every cell found */
        virtual void Execute() = 0;

    protected:
        Room* m_room;
        BaseCellVisitor& m_cellVisitor;
};

/*********************************
 * Derived grid searchers section
 *********************************/

/* Checks near cells, which are still visible by WorldObject */
class NearObjectVisibilityGridSearcher : public BaseGridSearcher
{
    public:
        NearObjectVisibilityGridSearcher(Room* room, BaseCellVisitor &visitor, WorldObject* subject) : BaseGridSearcher(room, visitor), m_subject(subject) { };

        void Execute() override;

    private:
        WorldObject* m_subject;
};

/* Checks near cells, which are still visible by WorldObject */
class NearVisibilityGridSearcher : public BaseGridSearcher
{
    public:
        NearVisibilityGridSearcher(Room* room, BaseCellVisitor &visitor, uint32_t cellX, uint32_t cellY) : BaseGridSearcher(room, visitor), m_cellX(cellX), m_cellY(cellY) { };

        void Execute() override;

    private:
        uint32_t m_cellX, m_cellY;
};

/*********************************
 * Derived cell visitors section
 *********************************/

/* Visits cell and builds create block of every object in it */
class AllObjectCreateCellVisitor : public BaseCellVisitor
{
    public:
        AllObjectCreateCellVisitor(GamePacket &gp) : m_targetPacket(gp), m_counter(0) { };

        void Visit(Cell* cell) override;

        /* Retrieves count of objects visited */
        uint32_t GetCounter();

    private:
        GamePacket &m_targetPacket;
        uint32_t m_counter;
};

/* Visits cell and builds create block of every player in it */
class AllPlayerCreateCellVisitor : public BaseCellVisitor
{
    public:
        AllPlayerCreateCellVisitor(GamePacket &gp) : m_targetPacket(gp), m_counter(0) { };

        void Visit(Cell* cell) override;

        /* Retrieves count of players visited */
        uint32_t GetCounter();

    private:
        GamePacket &m_targetPacket;
        uint32_t m_counter;
};

/* Visits cell and broadcasts packet to every player in cell */
class BroadcastPacketCellVisitor : public BaseCellVisitor
{
    public:
        BroadcastPacketCellVisitor(GamePacket &gp) : m_targetPacket(gp) { };

        void Visit(Cell* cell) override;

    private:
        GamePacket &m_targetPacket;
};

#endif
