#include "General.h"
#include "Player.h"
#include "Network.h"
#include "Session.h"
#include "Opcodes.h"
#include "PacketHandlers.h"
#include "Log.h"
#include "StatusCodes.h"
#include "Helpers.h"
#include "sha1.h"
#include <string>

Session::Session(Player* plr) : m_player(plr)
{
    m_violationCounter = 0;
    m_remoteAddr = "UNKNOWN";
    m_latency = 0;
    m_lastPingSendTime = 0;
    m_isExpired = false;
    m_sessionTimeout = 0;
    m_pingWaitingResponse = false;
}

Session::~Session()
{
    //
}

void Session::Update(uint32_t diff)
{
    uint32_t msnow = getMSTime();

    // send ping packet if needed
    if (!m_pingWaitingResponse && getMSTimeDiff(m_lastPingSendTime, msnow) > PING_TIMER)
    {
        m_lastPingSendTime = msnow;

        m_pingWaitingResponse = true;

        // send ping packet
        GamePacket pingpacket(SP_PING);
        sNetwork->SendPacket(this, pingpacket);
    }
    else if (m_pingWaitingResponse && getMSTimeDiff(m_lastPingSendTime, msnow) > PING_RESPONSE_TIME_LIMIT)
    {
        // do not allow termination once again
        m_pingWaitingResponse = false;

        // schedule session expiry, if not already scheduled
        if (!GetSessionTimeoutValue())
            SetSessionTimeoutValue(SESSION_INACTIVITY_EXPIRE);
    }
}

void Session::HandlePacket(GamePacket &packet)
{
    // do not handle opcodes higher than maximum
    if (packet.GetOpcode() >= OPCODE_MAX)
    {
        sLog->Error("Client (IP: %s) sent invalid packet (unknown opcode %u), not handling", GetRemoteAddr(), packet.GetOpcode());
        IncreaseViolationCounter();
        return;
    }

    sLog->Debug("NETWORK: Received packet %u", packet.GetOpcode());

    // packet handlers might throw exception about trying to reach out of packet data range
    try
    {
        // verify the state of client connection
        if ((PacketHandlerTable[packet.GetOpcode()].stateRestriction & (1 << m_connectionState)) == 0)
        {
            sLog->Error("Client (IP: %s) sent invalid packet (opcode %u) for state %u, not handling", GetRemoteAddr(), packet.GetOpcode(), m_connectionState);
            IncreaseViolationCounter();
            return;
        }

        // look handler up in handler table and call it
        PacketHandlerTable[packet.GetOpcode()].handler(this, packet);

        // after successfull handling of packet, decrease violations count
        DecreaseViolationCounter();
    }
    catch (PacketReadException &ex)
    {
        sLog->Error("Read error during executing handler for opcode %u - attempt to read %u bytes at offset %u (real size %u bytes) (client IP: %s)", packet.GetOpcode(), ex.GetAttemptSize(), ex.GetPosition(), packet.GetSize(), GetRemoteAddr());
        IncreaseViolationCounter();
    }
}

Player* Session::GetPlayer()
{
    return m_player;
}

void Session::OverridePlayer(Player* pl, const char* sessionKey)
{
    // old player will be destroyed elsewhere, but only by networking thread

    m_player = pl;

    // override session key if necessary
    if (sessionKey)
        m_sessionKey = sessionKey;
}

void Session::SetConnectionInfo(SOCK socket, sockaddr_in &addr, char* remoteAddr)
{
    m_socket = socket;
    m_sockAddr = addr;

    if (remoteAddr)
        m_remoteAddr = remoteAddr;
}

void Session::SetConnectionState(ConnectionState cstate)
{
    m_connectionState = cstate;
}

ConnectionState Session::GetConnectionState()
{
    return m_connectionState;
}

SOCK Session::GetSocket()
{
    return m_socket;
}

sockaddr_in const& Session::GetSockAddr()
{
    return m_sockAddr;
}

const char* Session::GetRemoteAddr()
{
    return m_remoteAddr.c_str();
}

time_t Session::GetSessionTimeoutValue()
{
    return m_sessionTimeout;
}

void Session::SetSessionTimeoutValue(time_t tm)
{
    m_sessionTimeout = time(nullptr) + tm;
}

void Session::IncreaseViolationCounter()
{
    m_violationCounter++;

    if (m_violationCounter >= MAX_SESSION_VIOLATIONS)
    {
        sLog->Error("Client (IP: %s) exceeded maximum count of violations in network communication, disconnecting", GetRemoteAddr());
        m_isExpired = true;
    }
}

void Session::DecreaseViolationCounter()
{
    if (m_violationCounter > 0)
        m_violationCounter--;
}

void Session::ClearViolationCounter()
{
    m_violationCounter = 0;
}

bool Session::IsMarkedAsExpired()
{
    return m_isExpired;
}

void Session::Kick()
{
    // send kick packet
    GamePacket pkt(SP_KICK);
    pkt.WriteUInt8(STATUS_PLAYEREXIT_REPEATED_LOGIN); // reason
    sNetwork->SendPacket(this, pkt);

    // will be cut by network thread in next turn
    m_isExpired = true;
}

const char* Session::CreateSessionKey()
{
    unsigned char resbuf[64];
    memset(resbuf, 0, sizeof(resbuf));
    char hexbuf[64];
    memset(hexbuf, 0, sizeof(hexbuf));

    // prepare hash base
    std::string base = std::to_string(rand() % 2000) + std::string(m_player->GetName()) + std::to_string(rand() % 2000);

    sha1::calc(base.c_str(), base.length(), resbuf);
    sha1::toHexString(resbuf, hexbuf);

    // store 24char limited session key
    m_sessionKey = std::string(hexbuf).substr(0, 24);

    return m_sessionKey.c_str();
}

const char* Session::GetSessionKey()
{
    return m_sessionKey.c_str();
}

void Session::SignalLatencyMeasure()
{
    m_latency = getMSTimeDiff(m_lastPingSendTime, getMSTime());

    m_pingWaitingResponse = false;
}

uint32_t Session::GetLatency()
{
    return m_latency;
}
