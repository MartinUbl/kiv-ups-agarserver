#include "General.h"
#include "PacketHandlers.h"
#include "Session.h"
#include "Player.h"
#include "Storage.h"
#include "Opcodes.h"
#include "StatusCodes.h"
#include "sha1.h"
#include "Version.h"
#include "Helpers.h"

void PacketHandlers::Handle_NULL(Session* sess, GamePacket& packet)
{
    // NULL handler - this means we throw away whole packet
}

void PacketHandlers::Handle_ServerSide(Session* sess, GamePacket& packet)
{
    // This should never happen - we should never receive server-to-client packet
}

void PacketHandlers::HandleLoginRequest(Session* sess, GamePacket& packet)
{
    std::string username, password;
    uint32_t version;

    // read contents
    username = packet.ReadString();
    password = packet.ReadString();
    version = packet.ReadUInt32();

    // prepare response packet
    GamePacket resp(SP_LOGIN_RESPONSE, 1);
    uint8_t statusCode = STATUS_LOGIN_OK;

    // retrieve user record from database
    StorageResult::UserRecord* user = sStorage->GetUserByUsername(username.c_str());
    // user does not exist
    if (!user)
        statusCode = STATUS_LOGIN_INVALID_USER;
    // version does not match expected value
    else if (version != GAME_VERSION)
        statusCode = STATUS_LOGIN_VERSION_MISMATCH;
    else
    {
        // prepare SHA1 hash of received password
        unsigned char resbuf[64];
        memset(resbuf, 0, sizeof(resbuf));
        char hexbuf[64];
        memset(hexbuf, 0, sizeof(hexbuf));

        sha1::calc(password.c_str(), password.length(), resbuf);
        sha1::toHexString(resbuf, hexbuf);

        // passwords does not match the stored one
        if (strcmp((const char*)hexbuf, user->passwordHash) != 0)
            statusCode = STATUS_LOGIN_WRONG_PASSWORD;
    }

    if (statusCode == STATUS_LOGIN_OK)
    {
        // TODO: move connection state
    }

    // send response
    resp.WriteUInt8(statusCode);
    sNetwork->SendPacket(sess, resp);
    delete user;
}

void PacketHandlers::HandleRegisterRequest(Session* sess, GamePacket& packet)
{
    std::string username, password;
    uint32_t version;

    // read contents
    username = packet.ReadString();
    password = packet.ReadString();
    version = packet.ReadUInt32();

    // prepare response packet
    GamePacket resp(SP_LOGIN_RESPONSE, 1);
    uint8_t statusCode = STATUS_REGISTER_OK;

    if (username.length() < 4)
        statusCode = STATUS_REGISTER_NAME_TOO_SHORT;
    else if (username.length() > 32)
        statusCode = STATUS_REGISTER_NAME_TOO_LONG;
    else if (password.length() < 4)
        statusCode = STATUS_REGISTER_PASSWORD_TOO_SHORT;
    else if (password.length() > 32)
        statusCode = STATUS_REGISTER_PASSWORD_TOO_LONG;
    else if (!IsValidUsername(username.c_str()))
        statusCode = STATUS_REGISTER_INVALID_NAME;
    else if (version != GAME_VERSION)
        statusCode = STATUS_REGISTER_VERSION_MISMATCH;
    else
    {
        StorageResult::UserRecord* user = sStorage->GetUserByUsername(username.c_str());
        if (user)
        {
            statusCode = STATUS_REGISTER_NAME_IS_TAKEN;
            delete user;
        }
        else
        {
            // prepare SHA1 hash of received password
            unsigned char resbuf[64];
            memset(resbuf, 0, sizeof(resbuf));
            char hexbuf[64];
            memset(hexbuf, 0, sizeof(hexbuf));

            sha1::calc(password.c_str(), password.length(), resbuf);
            sha1::toHexString(resbuf, hexbuf);

            sStorage->StoreUser(username.c_str(), (const char*)hexbuf);
        }
    }

    resp.WriteUInt8(statusCode);
    sNetwork->SendPacket(sess, resp);
}
