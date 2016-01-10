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

        /* Sets visitor parameter */
        void SetParameter(int32_t param);

    protected:
        int32_t m_parameter;
};

/* Base for all grid searchers */
class BaseGridSearcher
{
    public:
        BaseGridSearcher(Room* room, BaseCellVisitor *visitor) : m_room(room), m_cellVisitor(visitor) { };

        /* Proceeds to search and visit every cell found */
        virtual void Execute() = 0;

    protected:
        Room* m_room;
        BaseCellVisitor* m_cellVisitor;
};

/*********************************
 * Derived grid searchers section
 *********************************/

/* Checks near cells, which are still visible by WorldObject */
class NearObjectVisibilityGridSearcher : public BaseGridSearcher
{
    public:
        NearObjectVisibilityGridSearcher(Room* room, BaseCellVisitor *visitor, WorldObject* subject) : BaseGridSearcher(room, visitor), m_subject(subject) { };

        void Execute() override;

    private:
        WorldObject* m_subject;
};

/* Checks near cells, which are still visible by WorldObject */
class NearVisibilityGridSearcher : public BaseGridSearcher
{
    public:
        NearVisibilityGridSearcher(Room* room, BaseCellVisitor *visitor, uint32_t cellX, uint32_t cellY) : BaseGridSearcher(room, visitor), m_cellX(cellX), m_cellY(cellY) { };

        void Execute() override;

    private:
        uint32_t m_cellX, m_cellY;
};

/* Sends destroy to cells player left and create to newly entered cells */
class VisibilityChangeGridSearcher : public BaseGridSearcher
{
    public:
        VisibilityChangeGridSearcher(Room* room, BaseCellVisitor *visitor, uint32_t oldCellX, uint32_t oldCellY, uint32_t newCellX, uint32_t newCellY) : BaseGridSearcher(room, visitor),
            m_oldCellX(oldCellX), m_oldCellY(oldCellY), m_newCellX(newCellX), m_newCellY(newCellY) { };

        void Execute() override;

    private:
        uint32_t m_oldCellX, m_oldCellY, m_newCellX, m_newCellY;
};

/* Sends destroy to cells player left and create to newly entered cells */
class CellDiscoveryGridSearcher : public BaseGridSearcher
{
    public:
        CellDiscoveryGridSearcher(Room* room, BaseCellVisitor *visitor, uint32_t oldCellX, uint32_t oldCellY, uint32_t newCellX, uint32_t newCellY) : BaseGridSearcher(room, visitor),
            m_oldCellX(oldCellX), m_oldCellY(oldCellY), m_newCellX(newCellX), m_newCellY(newCellY) { };

        void Execute() override;

    private:
        uint32_t m_oldCellX, m_oldCellY, m_newCellX, m_newCellY;
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

/* Visits cell and broadcasts packet to every player in cell */
class ObjectFinderCellVisitor : public BaseCellVisitor
{
    public:
        ObjectFinderCellVisitor(uint32_t objectId, bool isPlayer) : m_isPlayer(isPlayer), m_objectId(objectId), m_object(nullptr) {};

        void Visit(Cell* cell) override;

        WorldObject* GetFoundObject() { return m_object; };

    private:
        bool m_isPlayer;
        uint32_t m_objectId;
        WorldObject* m_object;
};

/* Visits cell and broadcasts packet to every player in cell depending on its parameter set (see BaseCellVisitor method SetParameter) */
class MultiplexBroadcastPacketCellVisitor : public BaseCellVisitor
{
    public:
        MultiplexBroadcastPacketCellVisitor(GamePacket &pkt1, GamePacket &pkt2) : m_srcPacket1(pkt1), m_srcPacket2(pkt2) { };

        void Visit(Cell* cell) override;

    private:
        GamePacket &m_srcPacket1;
        GamePacket &m_srcPacket2;
};

/* Visits cell and broadcasts packet to every player in cell */
class ManhattanClosestCellVisitor : public BaseCellVisitor
{
    public:
        ManhattanClosestCellVisitor(Position const& src, uint32_t sourceSize, WorldObject* except) : m_sourcePos(src), m_exception(except),
            m_sourceSize(sourceSize), m_closest(nullptr), m_closestDistance(10000.0f) { };

        void Visit(Cell* cell) override;

        WorldObject* GetFoundObject();

    private:
        Position const& m_sourcePos;
        WorldObject* m_exception;
        uint32_t m_sourceSize;
        WorldObject* m_closest;
        float m_closestDistance;
};

#endif
