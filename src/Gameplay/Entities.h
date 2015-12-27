#ifndef AGAR_ENTITIES_H
#define AGAR_ENTITIES_H

#include "WorldObject.h"

class IdleFoodEntity : public WorldObject
{
    public:
        IdleFoodEntity();
        ~IdleFoodEntity();

        void OnEatenByPlayer(Player* plr);

    protected:
        //

    private:
        //
};

class BonusFoodEntity : public WorldObject
{
    public:
        BonusFoodEntity();
        ~BonusFoodEntity();

    protected:
        //

    private:
        //
};

class TrapEntity : public WorldObject
{
    public:
        TrapEntity();
        ~TrapEntity();

    protected:
        //

    private:
        //
};

#endif
