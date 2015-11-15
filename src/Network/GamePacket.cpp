#include "General.h"
#include "GamePacket.h"
#include "Log.h"

GamePacket::GamePacket() : m_opcode(0), m_size(0), m_data(nullptr), m_readPos(0)
{
    //
}

GamePacket::GamePacket(uint16_t opcode, uint16_t size) : m_opcode(opcode), m_size(size), m_data(nullptr), m_readPos(0)
{
    //
}

GamePacket::~GamePacket()
{
    // if there was any data, delete them to avoid memory leaks
    if (m_data)
        delete[] m_data;
}

void GamePacket::SetReadPos(uint16_t pos)
{
    // do not allow setting cursor outside of data range
    if (pos > m_size)
    {
        sLog->Error("Attempting to set read cursor of packet with opcode %u to position %u whilst having only size of %u", m_opcode, pos, m_size);
        return;
    }

    m_readPos = pos;
}

std::string GamePacket::ReadString()
{
    // we can detect only starting point being out of range at this time
    if (m_readPos >= m_size)
        throw new PacketReadException(m_readPos, 1);

    int i;

    // find zero, or end of packet
    for (i = m_readPos; i < m_size; i++)
    {
        if (m_data[i] == '\0')
            break;
    }

    // if we reached end without finding zero, that means, the string is not properly ended
    // or we just tried to read something, that's not string; by all means, this is errorneous state
    if (m_data[i] != '\0')
        throw new PacketReadException(m_readPos, m_size - m_readPos + 1);

    int oldReadPos = m_readPos;
    // set read position one character further to skip the zero termination
    m_readPos = i+1;

    // and return "substring"
    return std::string((const char*)&m_data[oldReadPos], i - oldReadPos);
}

uint32_t GamePacket::ReadUInt32()
{
    // disallow reading more bytes, than available
    if (m_readPos + 4 > m_size)
        throw new PacketReadException(m_readPos, 4);

    uint32_t toret;
    memcpy(&toret, &m_data[m_readPos], 4);
    m_readPos += 4;
    return toret;
}

int32_t GamePacket::ReadInt32()
{
    // disallow reading more bytes, than available
    if (m_readPos + 4 > m_size)
        throw new PacketReadException(m_readPos, 4);

    int32_t toret;
    memcpy(&toret, &m_data[m_readPos], 4);
    m_readPos += 4;
    return toret;
}

uint16_t GamePacket::ReadUInt16()
{
    // disallow reading more bytes, than available
    if (m_readPos + 2 > m_size)
        throw new PacketReadException(m_readPos, 2);

    uint16_t toret;
    memcpy(&toret, &m_data[m_readPos], 2);
    m_readPos += 2;
    return toret;
}

int16_t GamePacket::ReadInt16()
{
    // disallow reading more bytes, than available
    if (m_readPos + 2 > m_size)
        throw new PacketReadException(m_readPos, 2);

    int16_t toret;
    memcpy(&toret, &m_data[m_readPos], 2);
    m_readPos += 2;
    return toret;
}

uint8_t GamePacket::ReadUInt8()
{
    // disallow reading more bytes, than available
    if (m_readPos + 1 > m_size)
        throw new PacketReadException(m_readPos, 1);

    uint8_t toret;
    memcpy(&toret, &m_data[m_readPos], 1);
    m_readPos += 1;
    return toret;
}

int8_t GamePacket::ReadInt8()
{
    // disallow reading more bytes, than available
    if (m_readPos + 1 > m_size)
        throw new PacketReadException(m_readPos, 1);

    int8_t toret;
    memcpy(&toret, &m_data[m_readPos], 1);
    m_readPos += 1;
    return toret;
}

float GamePacket::ReadFloat()
{
    // disallow reading more bytes, than available
    if (m_readPos + 4 > m_size)
        throw new PacketReadException(m_readPos, 4);

    float toret;
    memcpy(&toret, &m_data[m_readPos], 4);
    m_readPos += 4;
    return toret;
}

void GamePacket::SetData(uint8_t* data)
{
    // clear previously stored data, if any
    if (m_data)
        delete[] m_data;

    m_data = data;
}

uint16_t GamePacket::GetOpcode()
{
    return m_opcode;
}

uint16_t GamePacket::GetSize()
{
    return m_size;
}
