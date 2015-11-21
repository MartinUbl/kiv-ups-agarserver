#include "General.h"
#include "PacketHandlers.h"
#include "Session.h"
#include "Player.h"
#include "Storage.h"
#include "Gameplay.h"
#include "Room.h"
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
    uint32_t version, playerId;
    uint8_t statusCode;

    // read contents
    username = packet.ReadString();
    password = packet.ReadString();
    version = packet.ReadUInt32();

    // prepare response packet
    GamePacket resp(SP_LOGIN_RESPONSE, 1);
    statusCode = STATUS_LOGIN_OK;
    playerId = 0;

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
        else
        {
            sess->GetPlayer()->SetId(user->id);
            sess->GetPlayer()->SetName(user->username);
            playerId = (uint32_t)user->id;
        }
    }

    // send response
    resp.WriteUInt8(statusCode);
    if (statusCode == STATUS_LOGIN_OK)
    {
        resp.WriteUInt32(playerId);

        // move connection state to "lobby" after logging in
        sess->SetConnectionState(CONNECTION_STATE_LOBBY);
    }

    sNetwork->SendPacket(sess, resp);
    delete user;
}

void PacketHandlers::HandleRegisterRequest(Session* sess, GamePacket& packet)
{
    std::string username, password;
    uint32_t version, playerId;
    uint8_t statusCode;

    // read contents
    username = packet.ReadString();
    password = packet.ReadString();
    version = packet.ReadUInt32();

    // prepare response packet
    GamePacket resp(SP_REGISTER_RESPONSE, 1);
    statusCode = STATUS_REGISTER_OK;
    playerId = 0;

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

            user = sStorage->GetUserByUsername(username.c_str());
            if (!user)
                statusCode = STATUS_REGISTER_INVALID_NAME;
            else
            {
                sess->GetPlayer()->SetId(user->id);
                sess->GetPlayer()->SetName(user->username);
                playerId = (uint32_t)user->id;
                delete user;
            }
        }
    }

    resp.WriteUInt8(statusCode);
    if (statusCode == STATUS_REGISTER_OK)
    {
        resp.WriteUInt32(playerId);

        // move connection state to "lobby" after registering
        sess->SetConnectionState(CONNECTION_STATE_LOBBY);
    }
    sNetwork->SendPacket(sess, resp);
}

void PacketHandlers::HandleRoomListRequest(Session* sess, GamePacket& packet)
{
    uint8_t gameType;
    Room* tmp;

    gameType = packet.ReadInt8();

    std::list<Room*> roomList;

    // fetch room list from gameplay singleton
    sGameplay->GetRoomList(roomList, gameType);

    // and build response
    GamePacket resp(SP_ROOM_LIST_RESPONSE, 4 + roomList.size() * (4 + 1 + 1 + 1));
    resp.WriteUInt32(roomList.size());
    for (std::list<Room*>::iterator itr = roomList.begin(); itr != roomList.end(); ++itr)
    {
        tmp = *itr;
        resp.WriteUInt32(tmp->GetId());
        resp.WriteUInt8(tmp->GetGameType());
        resp.WriteUInt8(tmp->GetPlayerCount());
        resp.WriteUInt8(tmp->GetCapacity());
    }
    sNetwork->SendPacket(sess, resp);
}

void PacketHandlers::HandleJoinRoomRequest(Session* sess, GamePacket& packet)
{
    uint32_t roomId;
    uint8_t spectator;

    roomId = packet.ReadUInt32();
    spectator = packet.ReadUInt8();

    Room* rm = sGameplay->GetRoom(roomId);

    GamePacket resp(SP_JOIN_ROOM_RESPONSE);

    uint8_t statusCode = STATUS_ROOMJOIN_OK;

    if (!rm)
        statusCode = STATUS_ROOMJOIN_FAILED_NO_SUCH_ROOM;
    else if (rm->GetPlayerCount() >= rm->GetCapacity())
        statusCode = STATUS_ROOMJOIN_FAILED_CAPACITY;
    else if (sess->GetPlayer()->GetRoomId() != 0)
        statusCode = STATUS_ROOMJOIN_FAILED_ALREADY_IN_ROOM;
    // TODO: spectators
    //else if (!rm->AllowSpectators() && spectator)
    //    statusCode = STATUS_ROOMJOIN_NO_SPECTATORS;
    else
    {
        rm->AddPlayer(sess->GetPlayer());
        // move player to game stage
        sess->SetConnectionState(CONNECTION_STATE_GAME);
    }

    resp.WriteUInt8(statusCode);
    resp.WriteUInt32(0); // TODO: chat channels

    sNetwork->SendPacket(sess, resp);
}

void PacketHandlers::HandleWorldRequest(Session* sess, GamePacket& packet)
{
    Room* plroom;

    plroom = sGameplay->GetRoom(sess->GetPlayer()->GetRoomId());
    if (!plroom)
    {
        // TODO: kick player
        return;
    }

    plroom->PlaceNewPlayer(sess->GetPlayer());

    GamePacket resp(SP_NEW_WORLD);

    // build this player update, so the client will know, where we are and how do we look like
    sess->GetPlayer()->BuildCreatePacketBlock(resp);

    // writes 4B player count and player create block for each player in range
    plroom->BuildPlayerCreateBlock(resp, sess->GetPlayer());
    // writes 4B object count and object create block for each object in range
    plroom->BuildObjectCreateBlock(resp, sess->GetPlayer());
    sNetwork->SendPacket(sess, resp);
}
