#ifndef AGAR_STATUSCODES_H
#define AGAR_STATUSCODES_H

/* Login status codes */
enum LoginStatusCode
{
    STATUS_LOGIN_OK = 0,
    STATUS_LOGIN_INVALID_USER = 1,
    STATUS_LOGIN_WRONG_PASSWORD = 2,
    STATUS_LOGIN_VERSION_MISMATCH = 3
};

/* Register status codes */
enum RegisterStatusCode
{
    STATUS_REGISTER_OK = 0,
    STATUS_REGISTER_INVALID_NAME = 1,
    STATUS_REGISTER_NAME_TOO_SHORT = 2,
    STATUS_REGISTER_NAME_TOO_LONG = 3,
    STATUS_REGISTER_PASSWORD_TOO_SHORT = 4,
    STATUS_REGISTER_PASSWORD_TOO_LONG = 5,
    STATUS_REGISTER_NAME_IS_TAKEN = 6,
    STATUS_REGISTER_VERSION_MISMATCH = 7
};

/* Room join result status codes */
enum RoomJoinCode
{
    STATUS_ROOMJOIN_OK = 0,
    STATUS_ROOMJOIN_FAILED_CAPACITY = 1,
    STATUS_ROOMJOIN_NO_SPECTATORS = 2,
    STATUS_ROOMJOIN_FAILED_NO_SUCH_ROOM = 3,
    STATUS_ROOMJOIN_FAILED_ALREADY_IN_ROOM = 4
};

/* Room create result status codes */
enum RoomCreateCode
{
    STATUS_ROOMCREATE_OK = 0,
    STATUS_ROOMCREATE_SERVER_LIMIT = 1,
    STATUS_ROOMCREATE_INVALID_PARAMETERS = 2
};

/* Bonus use result fail codes */
enum BonusUseFailCode
{
    STATUS_BONUSUSE_OVERLAP = 1,
    STATUS_BONUSUSE_WRONG_ZONE = 2,
    STATUS_BONUSUSE_NOTHING_TO_USE = 3
};

/* Player exit reason status codes */
enum PlayerExitCode
{
    STATUS_PLAYEREXIT_LEAVE = 0,
    STATUS_PLAYEREXIT_KICKED_AFK = 1,
    STATUS_PLAYEREXIT_KICKED_SUSPICIOUS = 2,
    STATUS_PLAYEREXIT_CONNECTION_ERROR = 3,
    STATUS_PLAYEREXIT_REPEATED_LOGIN = 4
};

/* Session restore status codes */
enum RestoreSessionCode
{
    STATUS_SESSIONREST_OK = 0,
    STATUS_SESSIONREST_FAILED_NOTFOUND = 1
};

#endif
