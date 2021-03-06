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
#include "GridSearchers.h"
#include "Log.h"

void PacketHandlers::Handle_NULL(Session* sess, GamePacket& packet)
{
    // NULL handler - this means we throw away whole packet
}

void PacketHandlers::Handle_ServerSide(Session* sess, GamePacket& packet)
{
    // This should never happen - we should never receive server-to-client packet
}

void PacketHandlers::HandlePong(Session* sess, GamePacket& packet)
{
    sess->SignalLatencyMeasure();

    GamePacket resp(SP_PING_PONG);
    resp.WriteUInt32(sess->GetLatency());

    sNetwork->SendPacket(sess, resp);
}

void PacketHandlers::HandleLoginRequest(Session* sess, GamePacket& packet)
{
    std::string username, password, sessionKey;
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

    sessionKey = sess->CreateSessionKey();

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
            // if there's another user logged in to that account, kick him
            Session* existing = sNetwork->FindSessionByPlayerId(user->id);
            if (existing)
            {
                Room* plroom = nullptr;
                if (existing->GetPlayer()->GetRoomId())
                    plroom = sGameplay->GetRoom(existing->GetPlayer()->GetRoomId());

                sLog->Info("Existing player: %u, timeout: %u, room: %u", user->id, existing->GetSessionTimeoutValue(), plroom ? 1 : 0);

                // Possible scenarios:
                // player is playing, somebody tries to login --> kick player
                // player is in lobby, somebody tries to login --> kick player
                // player is offline in lobby (session in timeout), somebody tries to login --> kick player
                // player is offline in game (session in timeout), somebody tries to login --> suggest session restore

                // if existing player does not have timeout interval or is not in any room, kick
                if (!existing->GetSessionTimeoutValue() || !plroom)
                    existing->Kick();
                else // otherwise suggest client to restore session
                {
                    statusCode = STATUS_LOGIN_SESSION_RESTORE;
                    sessionKey = existing->GetSessionKey();
                }
            }

            sess->GetPlayer()->SetId(user->id);
            sess->GetPlayer()->SetName(user->username);
            playerId = (uint32_t)user->id;
        }
    }

    // send response
    resp.WriteUInt8(statusCode);
    if (statusCode == STATUS_LOGIN_OK || statusCode == STATUS_LOGIN_SESSION_RESTORE)
    {
        resp.WriteUInt32(playerId);

        // move connection state to "lobby" after logging in
        if (statusCode != STATUS_LOGIN_SESSION_RESTORE)
            sess->SetConnectionState(CONNECTION_STATE_LOBBY);
    }

    // write session key in case of session restore
    resp.WriteString(sessionKey.c_str());

    sNetwork->SendPacket(sess, resp);
    delete user;

    // kick current session, the client will reset connection status, and start again
    // this is due to allow us to have generic flow
    if (statusCode == STATUS_LOGIN_SESSION_RESTORE)
        sess->Kick();
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

    // write session key in case of session restore
    resp.WriteString(sess->CreateSessionKey());

    sNetwork->SendPacket(sess, resp);
}

void PacketHandlers::HandleRestoreSession(Session* sess, GamePacket& packet)
{
    std::string sessionKey;
    uint32_t playerId;

    sessionKey = packet.ReadString();
    playerId = packet.ReadUInt32();

    sLog->Info("Attempting to restore session of player %u, sessionKey %s", playerId, sessionKey.c_str());

    GamePacket resp(SP_RESTORE_SESSION_RESPONSE);

    Session* existing = sNetwork->FindSessionBySessionKey(sessionKey.c_str());

    if (existing && playerId && existing->GetPlayer()->GetId() != playerId)
        existing = sNetwork->FindSessionByPlayerId(playerId);

    if (!existing)
    {
        resp.WriteUInt8(STATUS_SESSIONREST_FAILED_NOTFOUND);
        sLog->Error("Failed to restore session with key: %s", sessionKey.c_str());
    }
    else
    {
        resp.WriteUInt8(STATUS_SESSIONREST_OK);
        sLog->Info("Restored session with key: %s", sessionKey.c_str());

        uint32_t rid = existing->GetPlayer()->GetRoomId();
        resp.WriteUInt32(rid); // 0 = no room, other value = room ID

        if (rid)
        {
            sLog->Info("Player returned to room %u", rid);
            sess->SetConnectionState(CONNECTION_STATE_GAME);

            resp.WriteUInt32(0); // TODO: chat channels, this is chat channel ID
            // anything else?
        }
        else
        {
            sLog->Info("Player returned to lobby");
            sess->SetConnectionState(CONNECTION_STATE_LOBBY);
        }

        // switch session/player, so the NEW session is restored to OLD player
        // (old session is destroyed, as well, as new player record)

        Player* oldpl = existing->GetPlayer();
        Player* dummypl = sess->GetPlayer();
        Session* oldsess = oldpl->GetSession();

        // switch player records in update array (literally switch)
        sNetwork->OverridePlayerClient(dummypl, oldpl);

        // set this session to old player
        oldpl->OverrideSession(sess);
        // set dummy player to old session
        dummypl->OverrideSession(oldsess);

        // new session should contain old player
        sess->OverridePlayer(oldpl, oldsess->GetSessionKey());
        // and old session should contain new player (dummy)
        oldsess->OverridePlayer(dummypl);

        // destroy old session and new dummy player
        oldsess->Kick();

        // stop movement, if any
        oldpl->SetMoving(false);
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
        resp.WriteString(tmp->GetRoomName());
    }
    sNetwork->SendPacket(sess, resp);
}

void PacketHandlers::HandleJoinRoomRequest(Session* sess, GamePacket& packet)
{
    uint32_t roomId;
    /*uint8_t spectator;*/

    roomId = packet.ReadUInt32();
    /*spectator = */packet.ReadUInt8();

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
        sess->GetPlayer()->SetUpdateEnabled(false);

        rm->AddPlayer(sess->GetPlayer());
        // move player to game stage
        sess->SetConnectionState(CONNECTION_STATE_GAME);
    }

    resp.WriteUInt8(statusCode);
    resp.WriteUInt32(0); // TODO: chat channels

    sNetwork->SendPacket(sess, resp);
}

void PacketHandlers::HandleCreateRoom(Session* sess, GamePacket& packet)
{
    uint32_t size, capacity;
    std::string name;
    Room* rm = nullptr;
    uint8_t statusCode = STATUS_ROOMCREATE_OK;

    name = packet.ReadString();
    capacity = packet.ReadUInt32();
    size = packet.ReadUInt32();

    GamePacket resp(SP_JOIN_ROOM_RESPONSE);

    if (capacity > 50 || capacity < 2 || size > 500 || size < 20)
        statusCode = STATUS_ROOMCREATE_INVALID_PARAMETERS;
    else
    {
        rm = sGameplay->CreateRoom(GAME_TYPE_FREEFORALL, capacity, name.c_str(), size);

        if (!rm)
            statusCode = STATUS_ROOMCREATE_SERVER_LIMIT;
        else
        {
            sess->GetPlayer()->SetUpdateEnabled(false);

            rm->AddPlayer(sess->GetPlayer());
            // move player to game stage
            sess->SetConnectionState(CONNECTION_STATE_GAME);
        }
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
        sLog->Debug("Player %u is not in any room, kicking!", sess->GetPlayer()->GetId());

        // TODO: kick player
        return;
    }

    bool reinitGame = (packet.ReadUInt8() == 1);

    // if reinitializing, do not reset player state, just use current
    if (!reinitGame)
    {
        plroom->PlaceNewPlayer(sess->GetPlayer());
        sess->GetPlayer()->SetDead(false);
        sess->GetPlayer()->ResetAttributes();
    }

    GamePacket resp(SP_NEW_WORLD);

    // write map dimensions
    resp.WriteFloat(plroom->GetMapSizeX());
    resp.WriteFloat(plroom->GetMapSizeY());

    // build this player update, so the client will know, where we are and how do we look like
    sess->GetPlayer()->BuildCreatePacketBlock(resp);

    // writes 4B player count and player create block for each player in range
    plroom->BuildPlayerCreateBlock(resp, sess->GetPlayer());
    // writes 4B object count and object create block for each object in range
    plroom->BuildObjectCreateBlock(resp, sess->GetPlayer());

    sNetwork->SendPacket(sess, resp);

    sess->GetPlayer()->SetUpdateEnabled(true);
}

void PacketHandlers::HandleMoveStart(Session* sess, GamePacket& packet)
{
    // TODO: checks for too fast movement, etc.

    Player* plr = sess->GetPlayer();
    Room* plroom = sGameplay->GetRoom(plr->GetRoomId());
    float posX, posY, angle;

    posX = packet.ReadFloat();
    posY = packet.ReadFloat();
    angle = packet.ReadFloat();

    Position plpos(posX, posY);

    plr->Relocate(plpos, true);
    plr->SetMoveAngle(angle);
    plr->SetMoving(true);

    // broadcast packet about movement start
    GamePacket movestart(SP_MOVE_START);
    movestart.WriteUInt32(plr->GetId());
    movestart.WriteFloat(angle);

    BroadcastPacketCellVisitor visitor(movestart);
    NearObjectVisibilityGridSearcher gs(plroom, &visitor, plr);

    gs.Execute();
}

void PacketHandlers::HandleMoveStop(Session* sess, GamePacket& packet)
{
    Player* plr = sess->GetPlayer();
    Room* plroom = sGameplay->GetRoom(plr->GetRoomId());
    float posX, posY;

    posX = packet.ReadFloat();
    posY = packet.ReadFloat();
    packet.ReadFloat(); // angle, we don't use it (yet?)

    Position plpos(posX, posY);

    plr->Relocate(plpos, true);
    plr->SetMoving(false);

    // broadcast packet about movement stop
    GamePacket movestop(SP_MOVE_STOP);
    movestop.WriteUInt32(plr->GetId());
    movestop.WriteFloat(posX);
    movestop.WriteFloat(posY);

    BroadcastPacketCellVisitor visitor(movestop);
    NearObjectVisibilityGridSearcher gs(plroom, &visitor, plr);

    gs.Execute();
}

void PacketHandlers::HandleMoveHeartbeat(Session* sess, GamePacket& packet)
{
    // TODO: checks for too fast movement, etc.

    Player* plr = sess->GetPlayer();
    float posX, posY;

    posX = packet.ReadFloat();
    posY = packet.ReadFloat();

    Position plpos(posX, posY);

    plr->Relocate(plpos, true);
}

void PacketHandlers::HandleMoveDirection(Session* sess, GamePacket& packet)
{
    Player* plr = sess->GetPlayer();
    Room* plroom = sGameplay->GetRoom(plr->GetRoomId());
    float angle;

    angle = packet.ReadFloat();

    plr->SetMoveAngle(angle);

    // broadcast packet about movement stop
    GamePacket anglechange(SP_MOVE_DIRECTION);
    anglechange.WriteUInt32(plr->GetId());
    anglechange.WriteFloat(angle);

    BroadcastPacketCellVisitor visitor(anglechange);
    NearObjectVisibilityGridSearcher gs(plroom, &visitor, plr);

    gs.Execute();
}

void PacketHandlers::HandleEatRequest(Session* sess, GamePacket& packet)
{
    Player* plr = sess->GetPlayer();
    Room* plroom = sGameplay->GetRoom(plr->GetRoomId());

    uint8_t objType = packet.ReadUInt8();
    uint32_t objId = packet.ReadUInt32();

    bool isPlayer = (objType == PACKET_OBJECT_TYPE_PLAYER);

    ObjectFinderCellVisitor visitor(objId, isPlayer);
    NearObjectVisibilityGridSearcher gs(plroom, &visitor, plr);

    gs.Execute();

    WorldObject* obj = visitor.GetFoundObject();

    // TODO: maybe some nice "is this possible?" check

    if (obj)
    {
        if (isPlayer && ((Player*)obj)->GetSize() > plr->GetSize())
            plroom->EatObject((Player*)(obj), plr);
        else
            plroom->EatObject(plr, obj);
    }
}

void PacketHandlers::HandlePlayerExit(Session* sess, GamePacket& packet)
{
    Player* plr = sess->GetPlayer();
    Room* plroom = sGameplay->GetRoom(plr->GetRoomId());

    if (!plroom)
    {
        sLog->Error("Received player exit request when not in room!");
        return;
    }

    plroom->RemovePlayer(plr);
    plr->GetSession()->SetConnectionState(CONNECTION_STATE_LOBBY);

    // player exit packet was sent in RemovePlayer call, client exits room automatically
}

void PacketHandlers::HandleStatsRequest(Session* sess, GamePacket& packet)
{
    Player* plr = sess->GetPlayer();
    Room* plroom = sGameplay->GetRoom(plr->GetRoomId());

    if (!plroom)
    {
        sLog->Error("Received statistics request when not in room!");
        return;
    }

    GamePacket stats(SP_STATS_RESPONSE);
    plroom->BuildStatsBlock(stats);
    sNetwork->SendPacket(sess, stats);
}
