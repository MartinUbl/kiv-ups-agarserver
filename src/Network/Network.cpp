#include "General.h"
#include "GamePacket.h"
#include "Network.h"
#include "Log.h"
#include "Config.h"
#include "Session.h"
#include "Player.h"
#include "Helpers.h"
#include "Gameplay.h"
#include "Room.h"

Network::Network() : m_networkThread(nullptr)
{
    m_recvBytesCount = 0;
    m_sentBytesCount = 0;
    m_recvPacketsCount = 0;
    m_sentPacketsCount = 0;
}

Network::~Network()
{
    //
}

void runNetworkThread()
{
    sNetwork->Run();
}

bool Network::Startup()
{
    sLog->Info("Starting up networking...");

    // retrieve port number from config, and make sure we are dealing with valid port number
    int mp = sConfig->GetIntValue(CONF_PORT);
    // validate range
    if (mp < 0 || mp > MAX_VALID_NET_PORT)
    {
        sLog->Error("Invalid port %i specified, exiting", mp);
        return false;
    }
    // print warning when trying to run server on privileged (well-known) port
    if (mp <= MAX_PRIVILEGED_NET_PORT)
    {
        sLog->Error("It is not recommended to run this application on privileged port (%i); switch to port number between %u and %u", mp, MAX_PRIVILEGED_NET_PORT+1, MAX_VALID_NET_PORT);
    }

    // now we have valid port number
    m_port = (uint16_t)mp;

#ifdef _WIN32
    // on Windows, we need to start WinSock service first
    WORD version = MAKEWORD(1, 1);
    WSADATA data;
    if (WSAStartup(version, &data) != 0)
    {
        sLog->Error("Failed to start network service");
        return false;
    }
#endif

    // create socket as Internet TCP socket
    if ((m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        sLog->Error("Failed to create socket");
        return false;
    }

    int param = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&param, sizeof(int)) == -1)
    {
        sLog->Error("Failed to use SO_REUSEADDR flag, bind may fail due to orphan connections to old socket");
        // do not fail whole process, this is not mandatory
    }

    // retrieve address
    std::string bindAddr = sConfig->GetStringValue(CONF_BIND_IP);

    m_sockAddr.sin_family = AF_INET;
    m_sockAddr.sin_port = htons(m_port);

#ifdef _WIN32
    INET_PTON(AF_INET, bindAddr.c_str(), &m_sockAddr.sin_addr.s_addr);
#else
    m_sockAddr.sin_addr.s_addr = inet_addr(bindAddr.c_str());
#endif

    // detect invalid address supplied in config
    if (m_sockAddr.sin_addr.s_addr == INADDR_NONE)
    {
        sLog->Error("Invalid bind address %s specified. Please, specify valid IPv4 address", bindAddr.c_str());
        return false;
    }

    // bind to network interface/address
    if (bind(m_socket, (sockaddr*)&m_sockAddr, sizeof(m_sockAddr)) == -1)
    {
        sLog->Error("Failed to bind socket to %s:%u errno: %u", bindAddr.c_str(), m_port, LASTERROR());
        return false;
    }

    // create listen queue to be checked
    if (listen(m_socket, NETWORK_LISTEN_BACKLOG_SIZE) == -1)
    {
        sLog->Error("Couldn't create connection queue");
        return false;
    }

    // switch socket to nonblocking mode
#ifdef _WIN32
    u_long arg = WINSOCK_NONBLOCKING_ARG;
    if (ioctlsocket(m_socket, FIONBIO, &arg) == SOCKET_ERROR)
#else
    int oldFlag = fcntl(m_socket, F_GETFL, 0);
    if (fcntl(m_socket, F_SETFL, oldFlag | O_NONBLOCK) == -1)
#endif
    {
        sLog->Error("Failed to switch socket to non-blocking mode");
    }

    sLog->Info("Listening on %s:%u", bindAddr.c_str(), m_port);

    sLog->Info("Starting network thread");

    m_networkThread = new std::thread(runNetworkThread);

    sLog->Info("Networking started successfully!\n");

    return true;
}

void Network::Shutdown()
{
    sLog->Info("Shutting down networking...");

    SetRunningFlag(false);

    m_networkThread->join();

    sLog->Info("Networking thread stopped, closing socket");

    CloseSocket_gen(m_socket);
}

void Network::SetRunningFlag(bool state)
{
    std::unique_lock<std::mutex> lck(generic_mtx);

    m_isRunning = state;
}

bool Network::IsRunning()
{
    std::unique_lock<std::mutex> lck(generic_mtx);

    return m_isRunning;
}

void Network::Run()
{
    SetRunningFlag(true);

    // Main network update loop
    while (IsRunning())
    {
        sNetwork->Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Network::Update()
{
    // look into connection queue and accept new connections if any
    AcceptConnections();

    // if there are some clients, perform read, detect disconnections, etc.
    if (!m_clients.empty())
        UpdateClients();
}

void Network::AcceptConnections()
{
    SOCK res;
    int error;
    Player* plr;
    sockaddr_in accaddr;
    socklen_t addrlen = sizeof(accaddr);
    char tmpaddr[INET_ADDRSTRLEN];

    // try to accept incoming connection
    res = accept(m_socket, (sockaddr*)&accaddr, &addrlen);
    error = LASTERROR();

    // no valid connection
    if (res == INVALID_SOCKET)
    {
        // nonblocking socket returns "would block" state in error variable when no connection is
        // there to be accepted

        if (error != SOCKETWOULDBLOCK)
            sLog->Error("Socket error: %i", error);
    }
    else // this means we just accepted valid connection
    {
        INET_NTOP(AF_INET, &accaddr.sin_addr, tmpaddr, INET_ADDRSTRLEN);

        // create new player, set connection info to his session instance
        plr = new Player();
        plr->GetSession()->SetConnectionInfo(res, accaddr, tmpaddr);

#ifdef _WIN32
        u_long arg = WINSOCK_NONBLOCKING_ARG;
        if (ioctlsocket(res, FIONBIO, &arg) == SOCKET_ERROR)
#else
        int oldFlag = fcntl(m_socket, F_GETFL, 0);
        if (fcntl(res, F_SETFL, oldFlag | O_NONBLOCK) == -1)
#endif
        {
            sLog->Error("Failed to switch socket to non-blocking mode");
        }

        sLog->Debug("Accepting connection from: %s", tmpaddr);

        // insert into client list
        InsertClient(plr);
    }
}

void Network::CloseSocket_gen(SOCK socket)
{
#ifdef _WIN32
    shutdown(socket, SD_BOTH);
    closesocket(socket);
#else
    close(socket);
#endif
}

void Network::UpdateClients()
{
    uint16_t header_buf[2];
    int result;
    int error;
    Player* plr;
    Session* sess;
    uint8_t* recvdata;
    GamePacket pkt;
    time_t tmout;

    // go through all clients
    for (std::list<ClientRecord*>::iterator itr = m_clients.begin(); itr != m_clients.end(); )
    {
        plr = (*itr)->player;
        sess = plr->GetSession();

        // if session is marked for expiration, wait for it
        if (tmout = sess->GetSessionTimeoutValue())
        {
            if (tmout < time(nullptr))
                sess->Kick();
        }

        // if the session is marked as expired, disconnect client
        if (sess->IsMarkedAsExpired())
        {
            sLog->Debug("Client session (IP: %s) expired, disconnecting", sess->GetRemoteAddr());
            CloseSocket_gen(sess->GetSocket());
            itr = RemoveClient(itr);
            continue;
        }

        // try to read from socket assigned to client
        result = recv(sess->GetSocket(), (char*)&header_buf, GAMEPACKET_HEADER_SIZE, 0);
        error = LASTERROR();

        // some data available
        if (result > 0)
        {
            header_buf[0] = ntohs(header_buf[0]);
            header_buf[1] = ntohs(header_buf[1]);

            m_recvBytesCount += GAMEPACKET_HEADER_SIZE;

            // size read must be equal to header length
            if (result == GAMEPACKET_HEADER_SIZE && header_buf[1] < MAX_GAME_PACKET_SIZE)
            {
                // packet contents may be empty as well
                if (header_buf[1] > 0)
                {
                    // following memory is deallocated in GamePacket destructor, or in near error handler
                    recvdata = new uint8_t[header_buf[1]];
                    result = recv(sess->GetSocket(), (char*)recvdata, header_buf[1], 0);
                    error = LASTERROR();

                    // malformed packet - received less bytes than expected
                    if (result != (int)header_buf[1])
                    {
                        delete[] recvdata;

                        sLog->Error("Received malformed packet: opcode %u, size %u, real size %u; disconnecting client (IP: %s)", header_buf[0], header_buf[1], result, sess->GetRemoteAddr());
                        //CloseClientSocket(sess->GetSocket());
                        itr = RemoveClient(itr);
                        continue;
                    }

                    m_recvBytesCount += (int64_t)header_buf[1];
                }

                // build packet (this will cause previous packet destructor call and new packet constructor call)
                pkt = GamePacket(header_buf[0], header_buf[1]);

                m_recvPacketsCount++;

                // pass the data, if any
                if (header_buf[1] > 0)
                    pkt.SetData(recvdata, header_buf[1]);

                // and let the session handle the packet - cleanup is done in GamePacket destructor
                sess->HandlePacket(pkt);
            }
            else
            {
                sLog->Error("Received malformed packet: no valid headers sent; disconnecting client (IP: %s)", sess->GetRemoteAddr());
                CloseSocket_gen(sess->GetSocket());
                itr = RemoveClient(itr);
                continue;
            }
        }
        // connection abort, this may be due to network error
        else if (error == SOCKETCONNABORT)
        {
            // set timeout if necessary
            if (!sess->GetSessionTimeoutValue())
            {
                sess->SetSessionTimeoutValue(SESSION_INACTIVITY_EXPIRE);
                sLog->Error("Client (IP: %s) aborted connection, marking session as expired and waiting for timeout", sess->GetRemoteAddr());
            }
        }
        // connection closed by remote endpoint (either controlled or errorneous scenario, but initiated by client)
        else if (error == SOCKETCONNRESET)
        {
            sLog->Debug("Client (IP: %s) disconnected", sess->GetRemoteAddr());
            itr = RemoveClient(itr);
            continue;
        }
        else
        {
            // just check, if the error is caused by nonblocking socket, that would block, or that there's
            // no error at all (should not happen due to previous condition blocks, technically result == 0 means,
            // that we have some error set)
            if (error != SOCKETWOULDBLOCK && error != 0)
            {
                sLog->Error("Unhandled socket error: %u; disconnecting client (IP: %s)", error, sess->GetRemoteAddr());
                CloseSocket_gen(sess->GetSocket());
                itr = RemoveClient(itr);
                continue;
            }
        }

        ++itr;
    }
}

void Network::InsertClient(Player* plr)
{
    ClientRecord* cr = new ClientRecord;

    cr->player = plr;
    // defaulting connection state to "auth" since we need the player to log in first
    plr->GetSession()->SetConnectionState(CONNECTION_STATE_AUTH);

    m_clients.push_back(cr);
}

std::list<ClientRecord*>::iterator Network::RemoveClient(std::list<ClientRecord*>::iterator rec)
{
    // lookup client in rooms, to remove him from here
    if ((*rec)->player->GetRoomId() > 0)
    {
        Room* rm = sGameplay->GetRoom((*rec)->player->GetRoomId());
        rm->RemovePlayer((*rec)->player);
    }

    return m_clients.erase(rec);
}

void Network::SendPacket(Player* plr, GamePacket &pkt)
{
    SendPacket(plr->GetSession()->GetSocket(), pkt);
}

void Network::SendPacket(Session* sess, GamePacket &pkt)
{
    SendPacket(sess->GetSocket(), pkt);
}

void Network::SendPacket(SOCK socket, GamePacket &pkt)
{
    uint16_t op, sz;
    uint8_t* tosend = new uint8_t[GAMEPACKET_HEADER_SIZE + pkt.GetSize()];

    sLog->Debug("NETWORK: Sending packet %u", pkt.GetOpcode());

    op = htons(pkt.GetOpcode());
    sz = htons(pkt.GetSize());

    // write opcode
    memcpy(tosend, &op, 2);
    // write contents size
    memcpy(tosend + 2, &sz, 2);
    // write contents
    memcpy(tosend + 4, pkt.GetData(), pkt.GetSize());

    m_sentBytesCount += pkt.GetSize() + GAMEPACKET_HEADER_SIZE;
    m_sentPacketsCount++;

    // send response
    send(socket, (const char*)tosend, pkt.GetSize() + GAMEPACKET_HEADER_SIZE, MSG_NOSIGNAL);
}

Session* Network::FindSessionByPlayerId(uint32_t playerId)
{
    for (std::list<ClientRecord*>::iterator itr = m_clients.begin(); itr != m_clients.end(); ++itr)
    {
        if ((*itr)->player->GetId() == playerId)
            return (*itr)->player->GetSession();
    }

    return nullptr;
}

Session* Network::FindSessionBySessionKey(const char* sessionKey)
{
    for (std::list<ClientRecord*>::iterator itr = m_clients.begin(); itr != m_clients.end(); ++itr)
    {
        if (strcmp((*itr)->player->GetSession()->GetSessionKey(), sessionKey) == 0)
            return (*itr)->player->GetSession();
    }

    return nullptr;
}

void Network::OverridePlayerClient(Player* oldplayer, Player* newplayer)
{
    for (std::list<ClientRecord*>::iterator itr = m_clients.begin(); itr != m_clients.end(); ++itr)
    {
        // this will switch client records between old and new player records

        if ((*itr)->player == oldplayer)
            (*itr)->player = newplayer;
        else if ((*itr)->player == newplayer)
            (*itr)->player = oldplayer;
    }
}

uint64_t Network::GetRecvBytesCount()
{
    return m_recvBytesCount;
}

uint64_t Network::GetSentBytesCount()
{
    return m_sentBytesCount;
}

uint64_t Network::GetRecvPacketsCount()
{
    return m_recvPacketsCount;
}

uint64_t Network::GetSentPacketsCount()
{
    return m_sentPacketsCount;
}

