#include "processors/balanceTransReqProcessor.hpp"
#include "clients/lamportServer.hpp"

int BalanceTransReqProcessor::process(std::unique_ptr<Msg>&& msg) {
    auto lamport_client_ptr = LamportServer::getInstance();

    // Lock balance transaction mutex
    std::unique_lock<std::mutex> balance_lock(lamport_client_ptr->balance_mutex_);
    std::printf("[Server %d] Receive BalanceTransReq from server 0.\n", lamport_client_ptr->client_id_);

    // Generate BalanceTransRsp
    std::vector<std::pair<int, int>> client_balance_pairs = {};
    for (int i = 0; i < lamport_client_ptr->balance_tb_.size(); i++) {
        client_balance_pairs.push_back(std::make_pair(i + 1, lamport_client_ptr->balance_tb_[i])); // client_id = i + 1
    }
    std::unique_ptr<BalanceTransRsp> balance_trans_rsp_ptr = std::make_unique<BalanceTransRsp>(client_balance_pairs, lamport_client_ptr->client_id_);
    
    // Lock transfer priority queue mutex
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->transfer_pq_mutex_);
    lamport_client_ptr->blockchain_.printBlockchain();
    transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex

    // Stringify BalanceTransRsp
    std::printf("[Server %d] Send BalanceTransRsp to server 0. client_id: 1, balance: %d; client_id: 2, balance: %d; client_id: 3, balance: %d.\n", lamport_client_ptr->client_id_, lamport_client_ptr->balance_tb_[0], lamport_client_ptr->balance_tb_[1], lamport_client_ptr->balance_tb_[2]);
    auto parser = lamport_client_ptr->parser_factory_.createParser("BalanceTransRsp");
    std::string str = parser->stringify(std::move(balance_trans_rsp_ptr));
    
    // Send message to server 0
    lamport_client_ptr->sendMsg(0, str);

    balance_lock.unlock(); // Unlock balance transaction mutex
    return 0;
} 