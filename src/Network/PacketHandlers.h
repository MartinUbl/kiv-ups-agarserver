#ifndef AGAR_PACKETHANDLERS_H
#define AGAR_PACKETHANDLERS_H

#include "Session.h"
#include "GamePacket.h"

/* packet handler function arguments */
#define PACKET_HANDLER_ARGS Session* sess, GamePacket &packet
/* packez handler definition */
#define PACKET_HANDLER(x) void x(PACKET_HANDLER_ARGS)

/* structure of packet handler record */
struct PacketHandlerStructure
{
    /* handler function */
    void (*handler)(PACKET_HANDLER_ARGS);

    /* that's all for now, but in future, there may be some things like
       connection stage restrictions, etc. */
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
};

/* table of packet handlers; the opcode is also an index here */
static PacketHandlerStructure PacketHandlerTable[] = {
    { &PacketHandlers::Handle_NULL },               // OPCODE_NONE
    { &PacketHandlers::HandleLoginRequest },        // CP_LOGIN
    { &PacketHandlers::Handle_ServerSide },         // SP_LOGIN_RESPONSE
    { &PacketHandlers::HandleRegisterRequest },     // CP_REGISTER
    { &PacketHandlers::Handle_ServerSide },         // SP_REGISTER_RESPONSE
    { &PacketHandlers::HandleRoomListRequest },     // CP_ROOM_LIST
    { &PacketHandlers::Handle_ServerSide },         // SP_ROOM_LIST_RESPONSE
    { &PacketHandlers::HandleJoinRoomRequest },     // CP_JOIN_ROOM
    { &PacketHandlers::Handle_ServerSide },         // SP_JOIN_ROOM_RESPONSE
    { &PacketHandlers::Handle_NULL },               // CP_CREATE_ROOM
    { &PacketHandlers::Handle_ServerSide },         // SP_CREATE_ROOM_RESPONSE
    { &PacketHandlers::HandleWorldRequest },        // CP_WORLD_REQUEST
    { &PacketHandlers::Handle_ServerSide },         // SP_NEW_PLAYER
    { &PacketHandlers::Handle_ServerSide },         // SP_NEW_WORLD
    { &PacketHandlers::HandleMoveDirection },       // CP_MOVE_DIRECTION
    { &PacketHandlers::HandleMoveStart },           // CP_MOVE_START
    { &PacketHandlers::HandleMoveStop },            // CP_MOVE_STOP
    { &PacketHandlers::HandleMoveHeartbeat },       // CP_MOVE_HEARTBEAT
    { &PacketHandlers::Handle_ServerSide },         // SP_MOVE_DIRECTION
    { &PacketHandlers::Handle_ServerSide },         // SP_MOVE_START
    { &PacketHandlers::Handle_ServerSide },         // SP_MOVE_STOP
    { &PacketHandlers::Handle_ServerSide },         // SP_MOVE_HEARTBEAT
    { &PacketHandlers::Handle_ServerSide },         // SP_OBJECT_EATEN
    { &PacketHandlers::Handle_ServerSide },         // SP_PLAYER_EATEN
    { &PacketHandlers::Handle_NULL },               // CP_USE_BONUS
    { &PacketHandlers::Handle_ServerSide },         // SP_USE_BONUS_FAILED
    { &PacketHandlers::Handle_ServerSide },         // SP_USE_BONUS
    { &PacketHandlers::Handle_ServerSide },         // SP_CANCEL_BONUS
    { &PacketHandlers::Handle_ServerSide },         // SP_NEW_OBJECT
    { &PacketHandlers::Handle_NULL },               // CP_PLAYER_EXIT
    { &PacketHandlers::Handle_ServerSide },         // SP_PLAYER_EXIT
    { &PacketHandlers::Handle_NULL },               // CP_STATS
    { &PacketHandlers::Handle_ServerSide },         // SP_STATS_RESPONSE
    { &PacketHandlers::Handle_NULL },               // CP_CHAT_MSG
    { &PacketHandlers::Handle_ServerSide },         // SP_CHAT_MSG
    { &PacketHandlers::Handle_ServerSide },         // SP_DESTROY_OBJECT
    { &PacketHandlers::Handle_ServerSide },         // SP_UPDATE_WORLD
    { &PacketHandlers::HandleEatRequest },          // CP_EAT_REQUEST
};

#endif
