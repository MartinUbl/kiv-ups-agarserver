#include "General.h"
#include "Network.h"
#include "Session.h"
#include "Opcodes.h"
#include "PacketHandlers.h"
#include "Log.h"

Session::Session(Player* plr) : m_player(plr)
{
    m_violationCounter = 0;
    m_remoteAddr = "UNKNOWN";
}

Session::~Session()
{
    //
}

void Session::HandlePacket(GamePacket &packet)
{
    // do not handle opcodes higher than maximum
    if (packet.GetOpcode() >= OPCODE_MAX)
    {
        sLog->Error("Client sent invalid packet (unknown opcode %u), not handling", packet.GetOpcode());
        IncreaseViolationCounter();
        return;
    }

    //sLog->Debug("NETWORK: Received packet %u", packet.GetOpcode());

    // packet handlers might throw exception about trying to reach out of packet data range
    try
    {
        // verify the state of client connection
        if ((PacketHandlerTable[packet.GetOpcode()].stateRestriction & (1 << m_connectionState)) == 0)
        {
            sLog->Error("Client (IP: %s) sent invalid packet (opcode %u) for state %u, not handling", packet.GetOpcode(), m_connectionState, GetRemoteAddr());
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
