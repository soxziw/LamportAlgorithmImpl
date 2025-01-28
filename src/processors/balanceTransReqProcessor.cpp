#include "processors/balanceTransReqProcessor.hpp"
#include "clients/lamportClient.hpp"
#include "utils.hpp"

int BalanceTransReqProcessor::process(std::unique_ptr<Msg>&& msg) {
    auto lamport_client_ptr = LamportClient::getInstance();

    // Lock balance transaction mutex
    std::printf("[Client %d] Process BalanceTransReq.\n", lamport_client_ptr->getClientId());
    std::unique_lock<std::mutex> balance_lock(lamport_client_ptr->getBalanceMutex());

    // Generate BalanceTransRsp
    std::vector<std::pair<int, int>> client_balance_pairs = {};
    for (int i = 0; i < lamport_client_ptr->getBalanceTb().size(); i++) {
        client_balance_pairs.push_back(std::make_pair(i + 1, lamport_client_ptr->getBalanceTb()[i])); // client_id = i + 1
    }
    std::unique_ptr<BalanceTransRsp> balance_trans_rsp_ptr = std::make_unique<BalanceTransRsp>(client_balance_pairs, lamport_client_ptr->getClientId());
    
    // Lock transfer priority queue mutex
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->getTransferPqMutex());
    lamport_client_ptr->getBlockchain().printBlockchain();
    transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex

    // Stringify BalanceTransRsp
    std::printf("[Client %d] Send BalanceTransRsp to client 0. client_id: 1, balance: %d; client_id: 2, balance: %d; client_id: 3, balance: %d.\n", lamport_client_ptr->getClientId(), lamport_client_ptr->getBalanceTb()[0], lamport_client_ptr->getBalanceTb()[1], lamport_client_ptr->getBalanceTb()[2]);
    auto parser = lamport_client_ptr->getParserFactory().createParser("BalanceTransRsp");
    std::string str = parser->stringify(std::move(balance_trans_rsp_ptr));
    
    // Send message to client 0
    sendMsg(lamport_client_ptr, 0, str);

    balance_lock.unlock(); // Unlock balance transaction mutex
    return 0;
} 