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

Network::Network()
{
    //
}

Network::~Network()
{
    //
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
    // do not allow to run on privileged port range
    if (mp <= MAX_PRIVILEGED_NET_PORT)
    {
        sLog->Error("It is not allowed to run this application on privileged port (%i); switch to port number between %u and %u", mp, MAX_PRIVILEGED_NET_PORT+1, MAX_VALID_NET_PORT);
        return false;
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

    sLog->Info("Networking started successfully!\n");

    return true;
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
        // create new player, set connection info to his session instance
        plr = new Player();
        plr->GetSession()->SetConnectionInfo(res, accaddr);

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

        INET_NTOP(AF_INET, &accaddr.sin_addr, tmpaddr, INET_ADDRSTRLEN);
        sLog->Debug("Accepting connection from: %s", tmpaddr);

        // insert into client list
        InsertClient(plr);
    }
}

void Network::CloseClientSocket(SOCK socket)
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
    char tmpaddr[INET_ADDRSTRLEN];

    // go through all clients
    for (std::list<ClientRecord*>::iterator itr = m_clients.begin(); itr != m_clients.end(); )
    {
        plr = (*itr)->player;
        sess = plr->GetSession();

        // try to read from socket assigned to client
        result = recv(sess->GetSocket(), (char*)&header_buf, GAMEPACKET_HEADER_SIZE, 0);
        error = LASTERROR();

        // some data available
        if (result > 0)
        {
            header_buf[0] = ntohs(header_buf[0]);
            header_buf[1] = ntohs(header_buf[1]);

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

                        sLog->Error("Received malformed packet: opcode %u, size %u, real size %u; disconnecting client", header_buf[0], header_buf[1], result);
                        //shutdown
                        //CloseClientSocket(sess->GetSocket());
                        itr = RemoveClient(itr);
                        continue;
                    }
                }

                // build packet (this will cause previous packet destructor call and new packet constructor call)
                pkt = GamePacket(header_buf[0], header_buf[1]);

                // pass the data, if any
                if (header_buf[1] > 0)
                    pkt.SetData(recvdata, header_buf[1]);

                // and let the session handle the packet - cleanup is done in GamePacket destructor
                sess->HandlePacket(pkt);
            }
            else
            {
                sLog->Error("Received malformed packet: no valid headers sent; disconnecting client");
                CloseClientSocket(sess->GetSocket());
                itr = RemoveClient(itr);
                continue;
            }
        }
        // connection closed by remote endpoint (either controlled or errorneous scenario)
        else if (error == SOCKETCONNRESET)
        {
            INET_NTOP(AF_INET, (void*)&sess->GetSockAddr().sin_addr, tmpaddr, INET_ADDRSTRLEN);
            sLog->Debug("Client disconnected: %s", tmpaddr);
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
                sLog->Error("Unhandled socket error: %u; disconnecting client", error);
                CloseClientSocket(sess->GetSocket());
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

    //sLog->Debug("NETWORK: Sending packet %u", pkt.GetOpcode());

    op = htons(pkt.GetOpcode());
    sz = htons(pkt.GetSize());

    // write opcode
    memcpy(tosend, &op, 2);
    // write contents size
    memcpy(tosend + 2, &sz, 2);
    // write contents
    memcpy(tosend + 4, pkt.GetData(), pkt.GetSize());

    // send response
    send(socket, (const char*)tosend, pkt.GetSize() + GAMEPACKET_HEADER_SIZE, 0);
}
