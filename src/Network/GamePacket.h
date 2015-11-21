#ifndef AGAR_GAMEPACKET_H
#define AGAR_GAMEPACKET_H

#include <cstdint>
#include <exception>
#include <vector>

/* packet header size - 2B for opcode, 2B for size */
#define GAMEPACKET_HEADER_SIZE 4

/* Exception thrown when trying to read more than available remaining bytes */
class PacketReadException : public std::exception
{
    public:
        /* Only constructor we use to store values we need */
        PacketReadException(int pos, int attemptsize) : position(pos), attemptSize(attemptsize) { };

        /* Retrieves last valid position - the cursor position before read attempt */
        int GetPosition() { return position; };
        /* Retrieves the size we attempted to read */
        int GetAttemptSize() { return attemptSize; };

    private:
        /* cursor position before read attempt */
        int position;
        /* size we attempted to acquire */
        int attemptSize;
};

/* Class wrapping game packet header and contents */
class GamePacket
{
    public:
        /* default constructor, almost useless */
        GamePacket();
        /* constructor for known packet headers */
        GamePacket(uint16_t opcode, uint16_t size = 0);
        ~GamePacket();

        /* Sets data, typically when read from socket */
        void SetData(uint8_t* data, uint16_t size);
        /* Retrieves data array pointer */
        uint8_t* GetData();
        /* Retrieves packet opcode */
        uint16_t GetOpcode();
        /* Retrieves packet contents size (excluding header!) */
        uint16_t GetSize();

        /* Sets read cursor position */
        void SetReadPos(uint16_t pos);
        /* Retrieves location of write cursor */
        uint16_t GetWritePos();

        /* Reads zero-terminated string on current location */
        std::string ReadString();
        /* Reads 32bit unsigned integer on current location */
        uint32_t ReadUInt32();
        /* Reads 32bit signed integer on current location */
        int32_t ReadInt32();
        /* Reads 16bit unsigned short on current location */
        uint16_t ReadUInt16();
        /* Reads 16bit signed short on current location */
        int16_t ReadInt16();
        /* Reads 8bit unsigned byte on current location */
        uint8_t ReadUInt8();
        /* Reads 8bit signed byte on current location */
        int8_t ReadInt8();
        /* Reads 32bit floating point number on current location */
        float ReadFloat();

        /* Writes zero-terminated string on current location */
        void WriteString(const char* str);
        /* Writes 32bit unsigned integer on current location */
        void WriteUInt32(uint32_t val);
        /* Writes 32bit signed integer on current location */
        void WriteInt32(int32_t val);
        /* Writes 16bit unsigned integer on current location */
        void WriteUInt16(uint16_t val);
        /* Writes 16bit signed integer on current location */
        void WriteInt16(int16_t val);
        /* Writes 8bit unsigned integer on current location */
        void WriteUInt8(uint8_t val);
        /* Writes 8bit signed integer on current location */
        void WriteInt8(int8_t val);
        /* Writes 32bit floating point number on current location */
        void WriteFloat(float val);

        /* Writes 32bit unsigned integer at specified position */
        void WriteUInt32At(uint32_t val, uint16_t position);
        /* Writes 16bit unsigned integer at specified position */
        void WriteUInt16At(uint16_t val, uint16_t position);
        /* Writes 8bit unsigned integer at specified position */
        void WriteUInt8At(uint8_t val, uint16_t position);

    protected:
        /* Internal method for reading general data regardless of their type */
        void _Read(void* dst, size_t size);
        /* Internal method for writing general data regardless of their type */
        void _Write(void* data, size_t size);
        /* Internal method for writing general data regardless of their type on specified location */
        void _WriteAt(void* data, size_t size, uint16_t position);

        /* packet opcode */
        uint16_t m_opcode;
        /* contents size (excluding header) */
        uint16_t m_size;

        /* packet contents (excluding header) */
        std::vector<uint8_t> m_data;

        /* read cursor (points to first byte, that will be read by next Read* method) */
        uint16_t m_readPos;
        /* write cursor (points to first byte, that will be written by next Write* method) */
        uint16_t m_writePos;
};

#endif
