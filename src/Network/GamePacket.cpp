#include "General.h"
#include "GamePacket.h"
#include "Log.h"
#include "Helpers.h"

GamePacket::GamePacket() : m_opcode(0), m_size(0), m_readPos(0), m_writePos(0)
{
    //
}

GamePacket::GamePacket(uint16_t opcode, uint16_t size) : m_opcode(opcode), m_size(size), m_readPos(0), m_writePos(0)
{
    m_data.reserve(size);
}

GamePacket::~GamePacket()
{
    m_data.clear();
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

uint16_t GamePacket::GetWritePos()
{
    return m_writePos;
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

void GamePacket::_Read(void* dst, size_t size)
{
    // disallow reading more bytes than available
    if (m_readPos + size > m_size)
        throw new PacketReadException(m_readPos, m_size);

    memcpy(dst, &m_data[m_readPos], size);
    m_readPos += size;
}

uint32_t GamePacket::ReadUInt32()
{
    uint32_t toret;
    _Read(&toret, 4);
    return (uint32_t)ntohl(toret);
}

int32_t GamePacket::ReadInt32()
{
    int32_t toret;
    _Read(&toret, 4);
    return (int32_t)ntohl(toret);
}

uint16_t GamePacket::ReadUInt16()
{
    uint16_t toret;
    _Read(&toret, 2);
    return (uint16_t)ntohs(toret);
}

int16_t GamePacket::ReadInt16()
{
    int16_t toret;
    _Read(&toret, 2);
    return (int16_t)ntohs(toret);
}

uint8_t GamePacket::ReadUInt8()
{
    uint8_t toret;
    _Read(&toret, 1);
    return toret;
}

int8_t GamePacket::ReadInt8()
{
    int8_t toret;
    _Read(&toret, 1);
    return toret;
}

float GamePacket::ReadFloat()
{
    float toret;
    _Read(&toret, 4);
    return (float)ntohl((uint32_t)toret);
}

void GamePacket::_Write(void* data, size_t size)
{
    m_data.resize(m_writePos + 1 + size);
    memcpy(&m_data[m_writePos], data, size);
    m_writePos += size;

    if (m_size < m_writePos + 1)
        m_size = m_writePos + 1;
}

void GamePacket::_WriteAt(void* data, size_t size, uint16_t position)
{
    memcpy(&m_data[position], data, size);
}

void GamePacket::WriteString(const char* str)
{
    _Write((void*)str, strlen(str) + 1);
}

void GamePacket::WriteUInt32(uint32_t val)
{
    val = htonl(val);
    _Write(&val, sizeof(uint32_t));
}

void GamePacket::WriteInt32(int32_t val)
{
    val = htonl(val);
    _Write(&val, sizeof(int32_t));
}

void GamePacket::WriteUInt16(uint16_t val)
{
    val = htons(val);
    _Write(&val, sizeof(uint16_t));
}

void GamePacket::WriteInt16(int16_t val)
{
    val = htons((uint16_t)val);
    _Write(&val, sizeof(int16_t));
}

void GamePacket::WriteUInt8(uint8_t val)
{
    _Write(&val, sizeof(uint8_t));
}

void GamePacket::WriteInt8(int8_t val)
{
    _Write(&val, sizeof(int8_t));
}

void GamePacket::WriteFloat(float val)
{
    uint32_t cval = htonl(*(uint32_t*)&val);
    _Write(&cval, sizeof(float));
}

void GamePacket::WriteUInt32At(uint32_t val, uint16_t position)
{
    val = htonl(val);
    _WriteAt(&val, sizeof(uint32_t), position);
}

void GamePacket::WriteUInt16At(uint16_t val, uint16_t position)
{
    val = htons(val);
    _WriteAt(&val, sizeof(uint16_t), position);
}

void GamePacket::WriteUInt8At(uint8_t val, uint16_t position)
{
    _WriteAt(&val, sizeof(uint8_t), position);
}

void GamePacket::SetData(uint8_t* data, uint16_t size)
{
    m_data = std::vector<uint8_t>(data, data + size);
}

uint8_t* GamePacket::GetData()
{
    return m_data.data();
}

uint16_t GamePacket::GetOpcode()
{
    return m_opcode;
}

uint16_t GamePacket::GetSize()
{
    return m_size;
}
