#ifndef AGAR_OPCODES_H
#define AGAR_OPCODES_H

/*
 * Rule number one: when you need to add opcode, add it as next number in row,
 * DO NOT change numbers of opcodes, that are already defined, it may, and probably
 * will lead to further inconsistences.
 *
 * But, if you decide to not listen, and do whatever you want anyway, take look at
 * PacketHandlers.h, where handlers for packets are defined - opcodes are also key
 * in array, so the session handler could find the packet handler in O(1) time
 */

enum Opcodes
{
    OPCODE_NONE                 = 0x00,
    CP_LOGIN                    = 0x01,
    SP_LOGIN_RESPONSE           = 0x02,
    CP_REGISTER                 = 0x03,
    SP_REGISTER_RESPONSE        = 0x04,
    CP_ROOM_LIST                = 0x05,
    SP_ROOM_LIST_RESPONSE       = 0x06,
    CP_JOIN_ROOM                = 0x07,
    SP_JOIN_ROOM_RESPONSE       = 0x08,
    CP_CREATE_ROOM              = 0x09,
    SP_CREATE_ROOM_RESPONSE     = 0x0A,
    CP_WORLD_REQUEST            = 0x0B,
    SP_NEW_PLAYER               = 0x0C,
    SP_NEW_WORLD                = 0x0D,
    CP_MOVE_DIRECTION           = 0x0E,
    CP_MOVE_START               = 0x0F,
    CP_MOVE_STOP                = 0x10,
    CP_MOVE_HEARTBEAT           = 0x11,
    SP_MOVE_DIRECTION           = 0x12,
    SP_MOVE_START               = 0x13,
    SP_MOVE_STOP                = 0x14,
    SP_MOVE_HEARTBEAT           = 0x15,
    SP_OBJECT_EATEN             = 0x16,
    SP_PLAYER_EATEN             = 0x17,
    CP_USE_BONUS                = 0x18,
    SP_USE_BONUS_FAILED         = 0x19,
    SP_USE_BONUS                = 0x1A,
    SP_CANCEL_BONUS             = 0x1B,
    SP_NEW_OBJECT               = 0x1C,
    CP_PLAYER_EXIT              = 0x1D,
    SP_PLAYER_EXIT              = 0x1E,
    CP_STATS                    = 0x1F,
    SP_STATS_RESPONSE           = 0x20,
    CP_CHAT_MSG                 = 0x21,
    SP_CHAT_MSG                 = 0x22,
    SP_DESTROY_OBJECT           = 0x23,
    SP_UPDATE_WORLD             = 0x24,
    CP_EAT_REQUEST              = 0x25,
    SP_PING                     = 0x26,
    CP_PONG                     = 0x27,
    SP_PING_PONG                = 0x28,
    CP_RESTORE_SESSION          = 0x29,
    SP_RESTORE_SESSION_RESPONSE = 0x2A,
    SP_KICK                     = 0x2B,

    OPCODE_MAX
};

#endif
