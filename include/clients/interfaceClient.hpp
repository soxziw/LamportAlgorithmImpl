#pragma once

#include "clients/client.hpp"
#include "parsers/parserFactory.hpp"
#include "processors/processorFactory.hpp"
#include <map>

/**
 * InterfaceClient - client as interface.
 *
 * Users use this client as an local process interface to touch other clients.
 */
class InterfaceClient : public Client {
public:
    /**
     * init() - init interface client.
     *
     * @param client_id: id of current client.
     * @param ip_port_pairs: ip-port pairs for clients to build up socket connections.
     */
    void init(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs);

    /**
     * getInstance() - get the static/only instance of interface client.
     */
    static std::shared_ptr<InterfaceClient> getInstance();

    /**
     * terminate() - terminate interface client.
     */
    void terminate();

    /**
     * sendTransferTransReq() - send request of transfer transaction.
     *
     * @param client_id: client to send.
     * @param sender_id: sender of transfer.
     * @param receiver_id: receiver of transfer.
     * @param amount: amount of transfer.
     */
    int sendTransferTransReq(int client_id, int sender_id, int receiver_id, int amount);

    /**
     * sendBalanceTransReq() - send request of balance transaction.
     *
     * @param client_id: client to send.
     */
    int sendBalanceTransReq(int client_id);
};