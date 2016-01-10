#ifndef AGAR_PACKETHANDLERS_H
#define AGAR_PACKETHANDLERS_H

#include "Session.h"
#include "GamePacket.h"

/* packet handler function arguments */
#define PACKET_HANDLER_ARGS Session* sess, GamePacket &packet
/* packez handler definition */
#define PACKET_HANDLER(x) void x(PACKET_HANDLER_ARGS)

enum StateRestrictionMask
{
    STATE_RESTRICTION_NEVER         = 0,
    STATE_RESTRICTION_ANY           = (uint32_t)(-1),
    STATE_RESTRICTION_AUTH          = 1 << CONNECTION_STATE_AUTH,
    STATE_RESTRICTION_LOBBY         = 1 << CONNECTION_STATE_LOBBY,
    STATE_RESTRICTION_GAME          = 1 << CONNECTION_STATE_GAME,
    STATE_RESTRICTION_VERIFIED      = 1 << CONNECTION_STATE_LOBBY | 1 << CONNECTION_STATE_GAME,
};

/* structure of packet handler record */
struct PacketHandlerStructure
{
    /* handler function */
    void (*handler)(PACKET_HANDLER_ARGS);

    /* state restriction */
    StateRestrictionMask stateRestriction;
};

/* we wrap all packet handlers into namespace */
namespace PacketHandlers
{
    PACKET_HANDLER(Handle_NULL);
    PACKET_HANDLER(Handle_ServerSide);
    PACKET_HANDLER(HandleLoginRequest);
    PACKET_HANDLER(HandleRegisterRequest);
    PACKET_HANDLER(HandleRoomListRequest);
    PACKET_HANDLER(HandleJoinRoomRequest);
    PACKET_HANDLER(HandleWorldRequest);
    PACKET_HANDLER(HandleMoveStart);
    PACKET_HANDLER(HandleMoveStop);
    PACKET_HANDLER(HandleMoveHeartbeat);
    PACKET_HANDLER(HandleMoveDirection);
    PACKET_HANDLER(HandleEatRequest);
    PACKET_HANDLER(HandleRestoreSession);
    PACKET_HANDLER(HandlePong);
    PACKET_HANDLER(HandleCreateRoom);
    PACKET_HANDLER(HandlePlayerExit);
    PACKET_HANDLER(HandleStatsRequest);
};

/* table of packet handlers; the opcode is also an index here */
static PacketHandlerStructure PacketHandlerTable[] = {
    { &PacketHandlers::Handle_NULL,             STATE_RESTRICTION_NEVER },      // OPCODE_NONE
    { &PacketHandlers::HandleLoginRequest,      STATE_RESTRICTION_AUTH },       // CP_LOGIN
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_LOGIN_RESPONSE
    { &PacketHandlers::HandleRegisterRequest,   STATE_RESTRICTION_AUTH },       // CP_REGISTER
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_REGISTER_RESPONSE
    { &PacketHandlers::HandleRoomListRequest,   STATE_RESTRICTION_LOBBY },      // CP_ROOM_LIST
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_ROOM_LIST_RESPONSE
    { &PacketHandlers::HandleJoinRoomRequest,   STATE_RESTRICTION_LOBBY },      // CP_JOIN_ROOM
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_JOIN_ROOM_RESPONSE
    { &PacketHandlers::HandleCreateRoom,        STATE_RESTRICTION_LOBBY },      // CP_CREATE_ROOM
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_CREATE_ROOM_RESPONSE
    { &PacketHandlers::HandleWorldRequest,      STATE_RESTRICTION_GAME },       // CP_WORLD_REQUEST
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_NEW_PLAYER
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_NEW_WORLD
    { &PacketHandlers::HandleMoveDirection,     STATE_RESTRICTION_GAME },       // CP_MOVE_DIRECTION
    { &PacketHandlers::HandleMoveStart,         STATE_RESTRICTION_GAME },       // CP_MOVE_START
    { &PacketHandlers::HandleMoveStop,          STATE_RESTRICTION_GAME },       // CP_MOVE_STOP
    { &PacketHandlers::HandleMoveHeartbeat,     STATE_RESTRICTION_GAME },       // CP_MOVE_HEARTBEAT
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_MOVE_DIRECTION
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_MOVE_START
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_MOVE_STOP
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_MOVE_HEARTBEAT
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_OBJECT_EATEN
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_PLAYER_EATEN
    { &PacketHandlers::Handle_NULL,             STATE_RESTRICTION_GAME },       // CP_USE_BONUS
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_USE_BONUS_FAILED
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_USE_BONUS
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_CANCEL_BONUS
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_NEW_OBJECT
    { &PacketHandlers::HandlePlayerExit,        STATE_RESTRICTION_GAME },       // CP_PLAYER_EXIT
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_PLAYER_EXIT
    { &PacketHandlers::HandleStatsRequest,      STATE_RESTRICTION_GAME },       // CP_STATS
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_STATS_RESPONSE
    { &PacketHandlers::Handle_NULL,             STATE_RESTRICTION_VERIFIED },   // CP_CHAT_MSG
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_CHAT_MSG
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_DESTROY_OBJECT
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_UPDATE_WORLD
    { &PacketHandlers::HandleEatRequest,        STATE_RESTRICTION_GAME },       // CP_EAT_REQUEST
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_PING
    { &PacketHandlers::HandlePong,              STATE_RESTRICTION_ANY },        // CP_PONG
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_PING_PONG
    { &PacketHandlers::HandleRestoreSession,    STATE_RESTRICTION_ANY },        // CP_RESTORE_SESSION
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_RESTORE_SESSION_RESPONSE
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // SP_KICK
};

#endif
