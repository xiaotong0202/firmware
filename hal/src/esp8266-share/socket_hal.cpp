/******************************************************************************
Copyright (c) 2013-2015 IntoRobot Industries, Inc.  All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "socket/esp8266_conn.h"
#include "socket_hal.h"

Esp8266ConnClass esp8266_conn;

sock_handle_t socket_create(uint8_t family, uint8_t type, uint8_t protocol, uint16_t port, network_interface_t nif)
{
    sock_handle_t handle = esp8266_conn.socketCreate(protocol==IPPROTO_TCP ? MDM_IPPROTO_TCP : MDM_IPPROTO_UDP, port);
    return handle;
}

int32_t socket_connect(sock_handle_t sd, const sockaddr_t *addr, long addrlen)
{
    const uint8_t* addr_data = addr->sa_data;
    uint16_t port = addr_data[0]<<8 | addr_data[1];
    MDM_IP ip = IPADR(addr_data[2], addr_data[3], addr_data[4], addr_data[5]);
    bool result = esp8266_conn.socketConnect(sd, ip, port);
    return (result ? 0 : 1);
}

sock_result_t socket_reset_blocking_call()
{
    return 0;
}

sock_result_t socket_receive(sock_handle_t sd, void* buffer, socklen_t len, system_tick_t _timeout)
{
    sock_result_t result = 0;
    if (_timeout==0) {
        result = esp8266_conn.socketReadable(sd);
        if (result==0)// no data, so return without polling for data
            return 0;
        if (result>0)// clear error
            result = 0;
    }
    if (!result)
        result = esp8266_conn.socketRecv(sd, (char*)buffer, len);
    return result;
}

sock_result_t socket_create_nonblocking_server(sock_handle_t sock, uint16_t port)
{
    return 0;
}

sock_result_t socket_receivefrom(sock_handle_t sock, void* buffer, socklen_t bufLen, uint32_t flags, sockaddr_t* addr, socklen_t* addrsize)
{
    int port;
    MDM_IP ip;

    sock_result_t result = esp8266_conn.socketReadable(sock);
    if (result<=0)// error or no data
        return result;

    // have some data to let's get it.
    result = esp8266_conn.socketRecvFrom(sock, &ip, &port, (char*)buffer, bufLen);
    if (result > 0) {
        uint32_t ipv4 = ip;
        addr->sa_data[0] = (port>>8) & 0xFF;
        addr->sa_data[1] = port & 0xFF;
        addr->sa_data[2] = (ipv4 >> 24) & 0xFF;
        addr->sa_data[3] = (ipv4 >> 16) & 0xFF;
        addr->sa_data[4] = (ipv4 >> 8) & 0xFF;
        addr->sa_data[5] = ipv4 & 0xFF;
    }
    return result;
}

sock_result_t socket_bind(sock_handle_t sock, uint16_t port)
{
    return 0;
}

sock_result_t socket_accept(sock_handle_t sock)
{
    sock_handle_t handle = esp8266_conn.socketAccept(sock);
    return handle;
}

uint8_t socket_active_status(sock_handle_t socket)
{
    bool result = esp8266_conn.socketIsConnected(socket);
    return (result ? 0 : 1);
}

sock_result_t socket_close(sock_handle_t sock)
{
    bool result = esp8266_conn.socketFree(sock); // closes and frees the socket
    return (result ? 0 : 1);
}

sock_result_t socket_send(sock_handle_t sd, const void* buffer, socklen_t len)
{
    return esp8266_conn.socketSend(sd, (const char*)buffer, len);
}

sock_result_t socket_sendto(sock_handle_t sd, const void* buffer, socklen_t len, uint32_t flags, sockaddr_t* addr, socklen_t addr_size)
{
    const uint8_t* addr_data = addr->sa_data;
    uint16_t port = addr_data[0]<<8 | addr_data[1];
    MDM_IP ip = IPADR(addr_data[2], addr_data[3], addr_data[4], addr_data[5]);
    return esp8266_conn.socketSendTo(sd, ip, port, (const char*)buffer, len);
}

inline bool is_valid(sock_handle_t handle) {
    return esp8266_conn.socketIsValid(handle);
}

uint8_t socket_handle_valid(sock_handle_t handle) {
    return is_valid(handle);
}

sock_handle_t socket_handle_invalid()
{
    return esp8266_conn.socketInvalid();
}

sock_result_t socket_join_multicast(const HAL_IPAddress* addr, network_interface_t nif, void* reserved)
{
    return -1;
}

sock_result_t socket_leave_multicast(const HAL_IPAddress* addr, network_interface_t nif, void* reserved)
{
    return -1;
}

sock_result_t socket_peer(sock_handle_t sd, sock_peer_t* peer, void* reserved)
{
    return -1;
}

sock_result_t socket_create_tcp_server(uint16_t port, network_interface_t nif)
{
    sock_handle_t handle = esp8266_conn.socketCreate(MDM_IPPROTO_TCP_SERVER, port);
    return handle;
}

