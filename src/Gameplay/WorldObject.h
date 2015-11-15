#ifndef AGAR_WORLDOBJECT_H
#define AGAR_WORLDOBJECT_H

#include <math.h>

/* Wrapper structure for position in plane */
struct Position
{
    /* X coordinate */
    float x;
    /* Y coordinate */
    float y;

    /* Default constructor nulling all coordinates */
    Position() : x(0.0f), y(0.0f) { };
    /* Constructor with initial coordinates specified */
    Position(float mx, float my) : x(mx), y(my) { };

    /* Computes exact distance between two points on plane */
    float DistanceExact(Position &dst)
    {
        return sqrt(powf(x-dst.x, 2) + powf(y-dst.y, 2));
    }

    /* Computed Manhattan distance between two points on plane */
    float DistanceManhattan(Position &dst)
    {
        return fabs(x - dst.x) + fabs(y - dst.y);
    }
};

/* Class for identifying base object in world */
class WorldObject
{
    public:
        WorldObject();

        /* Retrieves object position */
        Position const& GetPosition();

    protected:
        /* Object position */
        Position m_position;

    private:
        //
};

#endif
