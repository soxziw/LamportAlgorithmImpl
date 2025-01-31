#include "clients/interfaceClient.hpp"


void InterfaceClient::init(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs) {
    initSockConfigs(client_id, ip_port_pairs);

    // Register parser for request/response of transfer/balance transaction
    std::printf("[Client %d] Register parser: TransferTransReqParser, BalanceTransReqParser, TransferTransRspParser, BalanceTransRspParser.\n", client_id);
    parser_factory_.registerParser("TransferTransReq", []() { return std::make_unique<TransferTransReqParser>(); });
    parser_factory_.registerParser("BalanceTransReq", []() { return std::make_unique<BalanceTransReqParser>(); });
    parser_factory_.registerParser("TransferTransRsp", []() { return std::make_unique<TransferTransRspParser>(); });
    parser_factory_.registerParser("BalanceTransRsp", []() { return std::make_unique<BalanceTransRspParser>(); });

    // Register processor for response of transfer/balance transanction
    std::printf("[Client %d] Register processor: TransferTransRspProcessor, BalanceTransRspProcessor.\n", client_id);
    processor_factory_.registerProcessor("TransferTransRsp", []() { return std::make_unique<TransferTransRspProcessor>(); });
    processor_factory_.registerProcessor("BalanceTransRsp", []() { return std::make_unique<BalanceTransRspProcessor>(); });
    
    // Start master and worker threads
    start();
}

std::shared_ptr<InterfaceClient> InterfaceClient::getInstance() {
    static std::shared_ptr<InterfaceClient> client_ptr; // static ensures uniqueness
    if (!client_ptr) {
        client_ptr = std::make_shared<InterfaceClient>();
    }
    return client_ptr;
}

void InterfaceClient::terminate() {
    // Stop master and worker threads
    stop();
}

int InterfaceClient::sendTransferTransReq(int client_id, int sender_id, int receiver_id, int amount) {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace([this, client_id, sender_id, receiver_id, amount] {
        // Stringify a TransferTransReq into string
        std::printf("[Client %d] Send TransferTransReq to client %d: %d pays %d $%d.\n", client_id_, client_id, sender_id, receiver_id, amount);
        std::unique_ptr<TransferTransReq> transfer_trans_req_ptr = std::make_unique<TransferTransReq>(sender_id, receiver_id, amount, 0);
        auto parser = this->parser_factory_.createParser("TransferTransReq");
        std::string str = parser->stringify(std::move(transfer_trans_req_ptr));

        this->sendMsg(client_id, str);
    });
    lock.unlock();
    cond_var_.notify_one();
    return 0;
}

int InterfaceClient::sendBalanceTransReq(int client_id) {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace([this, client_id] {
        // Stringify a BalanceTransReq into string
        std::printf("[Client %d] Send BalanceTransReq to client %d.\n", client_id_, client_id);
        std::unique_ptr<BalanceTransReq> balance_trans_req_ptr = std::make_unique<BalanceTransReq>(0);
        auto parser = this->parser_factory_.createParser("BalanceTransReq");
        std::string str = parser->stringify(std::move(balance_trans_req_ptr));

        this->sendMsg(client_id, str);
    });
    lock.unlock();
    cond_var_.notify_one();

    return 0;
}