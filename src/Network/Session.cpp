#include "General.h"
#include "Network.h"
#include "Session.h"
#include "Opcodes.h"
#include "PacketHandlers.h"
#include "Log.h"

Session::Session(Player* plr) : m_player(plr)
{
    //
}

Session::~Session()
{
    //
}

void Session::HandlePacket(GamePacket &packet)
{
    // do not handle opcodes higher than maximum
    if (packet.GetOpcode() >= OPCODE_MAX)
        return;

    // packet handlers might throw exception about trying to reach out of packet data range
    try
    {
        // look handler up in handler table and call it
        PacketHandlerTable[packet.GetOpcode()].handler(this, packet);
    }
    catch (PacketReadException &ex)
    {
        sLog->Error("Read error during executing handler for opcode %u - attempt to read %u bytes at offset %u (real size %u bytes)", packet.GetOpcode(), ex.GetAttemptSize(), ex.GetPosition(), packet.GetSize());
    }
}

Player* Session::GetPlayer()
{
    return m_player;
}

void Session::SetConnectionInfo(SOCK socket, sockaddr_in &addr)
{
    m_socket = socket;
    m_sockAddr = addr;
}

SOCK Session::GetSocket()
{
    return m_socket;
}

sockaddr_in const& Session::GetSockAddr()
{
    return m_sockAddr;
}
