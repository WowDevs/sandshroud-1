/*
 * Sandshroud Hearthstone
 * Copyright (C) 2004 - 2005 Antrix Team
 * Copyright (C) 2005 - 2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008 - 2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009 - 2010 Sandshroud <http://www.sandshroud.org/>
 * Copyright (C) 2010 - 2011 Sandshroud <http://www.sandshroud.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _LISTENSOCKET_H
#define _LISTENSOCKET_H
#ifdef CONFIG_USE_POLL

#include "SocketDefines.h"
#include <errno.h>

template<class T>
class ListenSocket
{
public:
    ListenSocket(const char * ListenAddress, uint32 Port)
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        SocketOps::ReuseAddr(m_socket);
        SocketOps::Nonblocking(m_socket);

        m_address.sin_family = AF_INET;
        m_address.sin_port = ntohs((u_short)Port);
        m_address.sin_addr.s_addr = htonl(INADDR_ANY);
        m_opened = false;

        if(strcmp(ListenAddress, "0.0.0.0"))
        {
            struct hostent * hostname = gethostbyname(ListenAddress);
            if(hostname != 0)
                memcpy(&m_address.sin_addr.s_addr, hostname->h_addr_list[0], hostname->h_length);
        }

        // bind.. well attempt to.
        int ret = bind(m_socket, (const sockaddr*)&m_address, sizeof(m_address));
        if(ret != 0)
        {
            printf("Bind unsuccessful on port %u.", Port);
            return;
        }

        ret = listen(m_socket, 5);
        if(ret != 0) 
        {
            printf("Unable to listen on port %u.", Port);
            return;
        }
        len = sizeof(sockaddr_in);
        m_opened = true;
    }

    ~ListenSocket()
    {
        if(m_opened)
            SocketOps::CloseSocket(m_socket);
    }

    void Close()
    {
        if(m_opened)
            SocketOps::CloseSocket(m_socket);
        m_opened = false;
    }

    void Update()
    {
        aSocket = accept(m_socket, (sockaddr*)&m_tempAddress, (socklen_t*)&len);
        if(aSocket == -1)
            return;

        dsocket = new T(aSocket);
        dsocket->Accept(&m_tempAddress);
    }

    inline bool IsOpen() { return m_opened; }

private:
    SOCKET m_socket;
    SOCKET aSocket;
    struct sockaddr_in m_address;
    struct sockaddr_in m_tempAddress;
    bool m_opened;
    uint32 len;
    T * dsocket;
};

#endif
#endif