#ifndef AGAR_NETWORK_H
#define AGAR_NETWORK_H

#include "Singleton.h"
#include "GamePacket.h"

#include <list>

/* Macro madness for main differences between Windows and Linux approach.
 * I personally need Windows-stuff because I use Windows for development.
 * All hail Microsoft.
 */
#ifdef _WIN32
 #include <Ws2tcpip.h>
 #define SOCK SOCKET
 #define ADDRLEN int

 #define SOCKETWOULDBLOCK WSAEWOULDBLOCK
 #define SOCKETCONNRESET  WSAECONNRESET
 #define SOCKETCONNABORT  WSAECONNABORTED
 #define LASTERROR() WSAGetLastError()
 #define INET_PTON(fam,addrptr,buff) InetPton(fam,addrptr,buff)
 #define INET_NTOP(fam,addrptr,buff,socksize) InetNtop(fam,addrptr,buff,socksize)
#else
 #include <iostream>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #include <string>
 #include <netdb.h>
 #include <fcntl.h>

 #define SOCK int
 #define ADDRLEN socklen_t

 #define INVALID_SOCKET -1

 #define SOCKETWOULDBLOCK EAGAIN
 #define SOCKETCONNABORT ECONNABORTED
 #define SOCKETCONNRESET ECONNRESET
 #define LASTERROR() errno
 #define INET_PTON(fam,addrptr,buff) inet_pton(fam,addrptr,buff)
 #define INET_NTOP(fam,addrptr,buff,socksize) inet_ntop(fam,addrptr,buff,socksize)
#endif

#ifndef MSG_NOSIGNAL
 #define MSG_NOSIGNAL 0
#endif

/* 1 kB is the maximum size allowed */
#define MAX_GAME_PACKET_SIZE 1024

/* privileged port range is from 0 to this number (usually 1023 as of IANA / IEEE spec) */
#define MAX_PRIVILEGED_NET_PORT 1023
/* maximal valid port number (since network port is 2 bytes long unsigned number, it's 2^16-1) */
#define MAX_VALID_NET_PORT 65535

/* size of listen queue for our socket */
#define NETWORK_LISTEN_BACKLOG_SIZE 10

/* wait this amount of seconds before kicking whole session */
#define SESSION_INACTIVITY_EXPIRE 60

/* WinSock nonblocking flag; this value is not defined in any WinSock headers, but is described as constant */
#define WINSOCK_NONBLOCKING_ARG 1

/* enumeration of allowed connection states */
enum ConnectionState
{
    CONNECTION_STATE_AUTH = 0,  // newly accepted connection, attempting to login/register
    CONNECTION_STATE_LOBBY = 1, // authenticated, but not playing atm.
    CONNECTION_STATE_GAME = 2,  // joined room and playing/spectating

    CONNECTION_STATE_MAX
};

class Player;
class Session;

/* Client record used when storing active player */
struct ClientRecord
{
    Player* player;

    // something more? bytes transferred? packets received/sent?
};

/* Networking singleton class */
class Network
{
    friend class Singleton<Network>;
    public:
        ~Network();

        /* Main method for running network updates */
        void Run();

        /* Shuts whole networking down */
        void Shutdown();

        /* Starts up networking, prepares everything needed to be run */
        bool Startup();
        /* Accepts new connections and processes messages/errors on currently estabilished ones */
        void Update();

        /* Sends packet to specific player */
        void SendPacket(Player* plr, GamePacket &pkt);
        /* Sends packet to specific session */
        void SendPacket(Session* sess, GamePacket &pkt);

        /* Finds session using player ID */
        Session* FindSessionByPlayerId(uint32_t playerId);
        /* Finds session using session key */
        Session* FindSessionBySessionKey(const char* sessionKey, Session* except = nullptr);

        /* Overrides player in client map */
        void OverridePlayerClient(Player* oldplayer, Player* newplayer);

        /* retrieves received bytes count */
        uint64_t GetRecvBytesCount();
        /* retrieves sent bytes count */
        uint64_t GetSentBytesCount();

        /* retrieves received packets count */
        uint64_t GetRecvPacketsCount();
        /* retrieves sent packets count */
        uint64_t GetSentPacketsCount();

    protected:
        /* Hidden singleton constructor */
        Network();

    private:
        /* Accept new connections if any */
        void AcceptConnections();
        /* Reads data from all sockets enlisted, detects connection problems, disconnections, etc. */
        void UpdateClients();

        /* Sets running flag */
        void SetRunningFlag(bool state);
        /* Is server still intended to run? */
        bool IsRunning();

        /* Sends game packet to specified socket */
        void SendPacket(SOCK socket, GamePacket &pkt);

        /* Closes client socket using OS-dependent routines */
        void CloseSocket_gen(SOCK socket);

        /* Inserts new client to internal list */
        void InsertClient(Player* plr);
        /* Removes existing client (using iterators, cause it's used inside list-iterating loop) */
        std::list<ClientRecord*>::iterator RemoveClient(std::list<ClientRecord*>::iterator rec);

        /* Server socket */
        SOCK m_socket;
        /* Server socket info */
        sockaddr_in m_sockAddr;
        /* Currently used port */
        unsigned short m_port;

        /* List of all connected clients */
        std::list<ClientRecord*> m_clients;

        /* instance of network thread */
        std::thread* m_networkThread;

        /* is server still intended to run? */
        bool m_isRunning;

        /* generic networking mutex */
        std::mutex generic_mtx;

        uint64_t m_recvBytesCount;
        uint64_t m_sentBytesCount;

        uint64_t m_recvPacketsCount;
        uint64_t m_sentPacketsCount;
};

#define sNetwork Singleton<Network>::getInstance()

#endif
