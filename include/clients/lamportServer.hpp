#pragma once

#include "clients/server.hpp"
#include "parsers/parserFactory.hpp"
#include "processors/processorFactory.hpp"
#include "blockchain.hpp"
#include <queue>

/**
 * Transfer - transfer transaction.
 */
class Transfer {
public:
    int sender_id = -1;
    int receiver_id = -1;
    int amount = 0;
    int lamport_clock = -1;
    int client_id = -1;
};

/**
 * TransferCompare - comparation class of transfer.
 *
 * In comparation, lamport_clock first, then client_id break the tie.
 */
class TransferCompare
{
public:
    bool operator() (const Transfer& t1, const Transfer& t2) const {
        if (t1.lamport_clock != t2.lamport_clock) {
            return t1.lamport_clock > t2.lamport_clock; /* lamport_clock first */
        } else {
            return t1.client_id > t2.client_id; /* client_id break the tie */
        }
    }
};

/**
 * LamportServer - server that implement lamport algorithm.
 */
class LamportServer : public Server {
public:
    /**
     * init() - init lamport server.
     *
     * @param client_id: id of current server.
     * @param balance_tb: initial balance table of all lamport clients.
     * @param ip_port_pairs: ip-port pairs for clients to build up socket connections.
     */
    void init(int client_id, const std::vector<int> balance_tb, const std::vector<std::pair<std::string, int>>& ip_port_pairs);

    /**
     * getInstance() - get the static/only instance of lamport server.
     */
    static std::shared_ptr<LamportServer> getInstance();

    /**
    * updateLamportClock() - merge local lamport clock with remote lamport clock on receiving message.
    *
    * @param remote_lamport_clock
    */
    int updateLamportClock(int remote_lamport_clock);

    /**
    * getLamportClock() - get a new local lamport clock on sending message.
    */
    int getLamportClock();

    /**
    * transfer() - implement top transfer.
    *
    * @param update_transfer_result: update transfer result of current server or not. not update on releasing.
    */
    void transfer(bool update_transfer_result);

    Blockchain blockchain_; /* local blockchain */
    std::vector<int> balance_tb_; /* local balance table */

    std::priority_queue<Transfer, std::vector<Transfer>, TransferCompare> transfer_pq_; /* local transfer priority queue */
    std::mutex transfer_pq_mutex_; /* mutex of transfer_pq_ */
    std::vector<bool> replys_; /* hearing replys from all other clients */
    std::mutex replys_mutex_; /* mutex for replys_ */
    std::vector<bool> finishes_; /* hearing finishes from all other clients */
    std::mutex finishes_mutex_; /* mutex for finishes_ */
    std::string transfer_result_; /* result of transfer transaction of current server */

    int lamport_clock_; /* local lamport clock */
    std::mutex lamport_clock_mutex_; /* mutex for lamport_clock_ */
    
    std::mutex balance_mutex_; /* mutex for balance transactions */
};