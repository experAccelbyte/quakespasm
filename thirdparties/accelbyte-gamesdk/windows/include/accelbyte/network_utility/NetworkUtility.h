// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// CNL
#include "accelbyte/network_utility/network_utility_exports.h"

// AccelByte
#include "accelbyte/common/Vector.h"
#include "accelbyte/common/Error.h"

namespace accelbyte {
namespace network_utility {

/**
 * @brief interface for a network utility
 **/
class AB_CNL_NETWORK_UTILITY_EXPORT NetworkUtility {
private:
    /** @defgroup tcp TCP only interface
     *  This group consists interfaces only for TCP
     *  @{
     */
    /**
     * @brief send data through TCP
     */
    virtual auto socket_send_tcp(const Vector<char>& data) -> accelbyte::Error = 0;

    /**
     * @brief receive data through TCP
     */
    virtual auto socket_receive_tcp(Vector<char>& data) -> accelbyte::Error = 0;
    /** @} */ // end of tcp

    /** @defgroup udp UDP only interface
     *  This group consists interfaces only for UDP
     *  @{
     */
    /**
     * @brief send data through UDP
     */
    virtual auto socket_send_udp(const Vector<char>& data) -> accelbyte::Error = 0;

    /**
     * @brief receive data through UDP
     */
    virtual auto socket_receive_udp(Vector<char>& data) -> accelbyte::Error = 0;
    /** @} */ // end of udp

public:
    enum class SocketProtocol : uint32_t { TCP, UDP };

    enum class NetworkType : uint32_t { CLIENT, SERVER };

    /**
     * @brief default destructor
     */
    virtual ~NetworkUtility() = default;

    /**
     * @brief socket library initialization
     */
    virtual auto init_socket_lib() -> accelbyte::Error = 0;

    /**
     * @brief setup the addrinfo (a struct containing the socket's information)
     */
    virtual auto setup_addrinfo() -> accelbyte::Error = 0;

    /**
     * @brief initialize socket
     */
    virtual auto init_socket() -> accelbyte::Error = 0;

    /**
     * @brief bind socket (Server only)
     */
    virtual auto bind_socket() -> accelbyte::Error = 0;

    /**
     * @brief socket send data
     */
    virtual auto socket_send(const Vector<char>& data) -> accelbyte::Error = 0;

    /**
     * @brief socket receive data
     */
    virtual auto socket_receive(Vector<char>& data) -> accelbyte::Error = 0;

    /**
     * @brief set socket timeout
     */
    virtual auto set_timeout(uint32_t miliseconds) -> void = 0;

    /**
     * @brief close opened socket
     */
    virtual auto close_socket() -> void = 0;

    /**
     * @brief get the max_buffer_size
     */
    virtual auto get_max_buffer_size() -> uint32_t = 0;

    /** @defgroup tcp TCP only interface
     *  This group consists the interfaces only for TCP
     *  @{
     */
    /**
     * @brief tell the socket to listen (waiting) for a connection (Server only)
     */
    virtual auto socket_listen() -> accelbyte::Error = 0;

    /**
     * @brief tell the socket to accept the connection (Server only)
     */
    virtual auto socket_accept() -> accelbyte::Error = 0;

    /**
     * @brief tell the socket to connect to a server (Client only)
     */
    virtual auto socket_connect() -> accelbyte::Error = 0;
    /** @} */ // end of tcp
};
} // namespace network_utility
} // namespace accelbyte
