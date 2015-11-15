#include "General.h"
#include "Player.h"
#include "Session.h"

Player::Player() : WorldObject()
{
    m_session = new Session(this);
}

Player::~Player()
{
    //
}

Session* Player::GetSession()
{
    return m_session;
}
