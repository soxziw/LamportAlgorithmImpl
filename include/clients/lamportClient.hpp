#pragma once

#include "clients/client.hpp"
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
 * LamportClient - client that implement lamport algorithm.
 */
class LamportClient : public Client {
public:
    /**
     * init() - init lamport client.
     *
     * @param client_id: id of current client.
     * @param balance_tb: initial balance table of all lamport clients.
     * @param ip_port_pairs: ip-port pairs for clients to build up socket connections.
     */
    void init(int client_id, const std::vector<int> balance_tb, const std::vector<std::pair<std::string, int>>& ip_port_pairs);

    /**
     * getInstance() - get the static/only instance of lamport client.
     */
    static std::shared_ptr<LamportClient> getInstance();

    /**
     * terminate() - terminate lamport client.
     */
    void terminate();

    /**
     * getBlockchain() - get blockchain_.
     */
    Blockchain& getBlockchain() {
        return blockchain_;
    }

    /**
     * getBalanceTb() - get balance_tb_.
     */
    std::vector<int>& getBalanceTb() {
        return balance_tb_;
    }

    /**
     * getTransferPq() - get transfer_pq_.
     */
    std::priority_queue<Transfer, std::vector<Transfer>, TransferCompare>& getTransferPq() {
        return transfer_pq_;
    }

    /**
     * getTransferPqMutex() - get transfer_pq_mutex_.
     */
    std::mutex& getTransferPqMutex() {
        return transfer_pq_mutex_;
    }

    /**
     * getReplys() - get replys_.
     */
    std::vector<bool>& getReplys() {
        return replys_;
    }

    /**
     * getReplysMutex() - get replys_mutex_.
     */
    std::mutex& getReplysMutex() {
        return replys_mutex_;
    }

    /**
     * getFinishes() - get finishes_.
     */
    std::vector<bool>& getFinishes() {
        return finishes_;
    }

    /**
     * getFinishesMutex() - get finishes_mutex_.
     */
    std::mutex& getFinishesMutex() {
        return finishes_mutex_;
    }

    /**
     * getTransferResult() - get transfer_result_.
     */
    std::string& getTransferResult() {
        return transfer_result_;
    }

    /**
     * getLamportClock() - get lamport_clock_.
     */
    int& getLamportClock() {
        return lamport_clock_;
    }

    /**
     * getLamportClockMutex() - get lamport_clock_mutex_.
     */
    std::mutex& getLamportClockMutex() {
        return lamport_clock_mutex_;
    }
   
    /**
     * getBalanceMutex() - get balance_mutex_.
     */ 
    std::mutex& getBalanceMutex() {
        return balance_mutex_;
    }

private:
    Blockchain blockchain_; /* local blockchain */
    std::vector<int> balance_tb_; /* local balance table */

    std::priority_queue<Transfer, std::vector<Transfer>, TransferCompare> transfer_pq_; /* local transfer priority queue */
    std::mutex transfer_pq_mutex_; /* mutex of transfer_pq_ */
    std::vector<bool> replys_; /* hearing replys from all other clients */
    std::mutex replys_mutex_; /* mutex for replys_ */
    std::vector<bool> finishes_; /* hearing finishes from all other clients */
    std::mutex finishes_mutex_; /* mutex for finishes_ */
    std::string transfer_result_; /* result of transfer transaction of current client */

    int lamport_clock_; /* local lamport clock */
    std::mutex lamport_clock_mutex_; /* mutex for lamport_clock_ */
    
    std::mutex balance_mutex_; /* mutex for balance transactions */
};