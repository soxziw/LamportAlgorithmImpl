#pragma once

#include "clients/server.hpp"
#include "parsers/parserFactory.hpp"
#include "processors/processorFactory.hpp"
#include <map>

/**
 * Interface - server as interface.
 *
 * Users use this server as an local process interface to touch other clients.
 */
class Interface : public Server {
public:
    /**
     * init() - init interface.
     *
     * @param client_id: id of current server.
     * @param ip_port_pairs: ip-port pairs for clients to build up socket connections.
     */
    void init(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs);

    /**
     * getInstance() - get the static/only instance of interface.
     */
    static std::shared_ptr<Interface> getInstance();

    /**
     * exit() - exit interface.
     */
    void exit();

    /**
     * sendExitMsg() - send exit messages to all other clients.
     */
    int sendExitMsg();

    /**
     * sendTransferTransReq() - send request of transfer transaction.
     *
     * @param client_id: server to send.
     * @param sender_id: sender of transfer.
     * @param receiver_id: receiver of transfer.
     * @param amount: amount of transfer.
     */
    int sendTransferTransReq(int client_id, int sender_id, int receiver_id, int amount);

    /**
     * sendBalanceTransReq() - send request of balance transaction.
     *
     * @param client_id: server to send.
     */
    int sendBalanceTransReq(int client_id);
};