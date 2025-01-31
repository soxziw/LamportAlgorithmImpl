#include "clients/interfaceClient.hpp"


void InterfaceClient::init(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs) {
    initSockConfigs(client_id, ip_port_pairs);

    // Register parser for request/response of transfer/balance transaction
    std::printf("[Client %d] Register parser: TransferTransReqParser, BalanceTransReqParser, TransferTransRspParser, BalanceTransRspParser, ExitMsgParser.\n", client_id);
    parser_factory_.registerParser("TransferTransReq", []() { return std::make_unique<TransferTransReqParser>(); });
    parser_factory_.registerParser("BalanceTransReq", []() { return std::make_unique<BalanceTransReqParser>(); });
    parser_factory_.registerParser("TransferTransRsp", []() { return std::make_unique<TransferTransRspParser>(); });
    parser_factory_.registerParser("BalanceTransRsp", []() { return std::make_unique<BalanceTransRspParser>(); });
    parser_factory_.registerParser("ExitMsg", []() { return std::make_unique<ExitMsgParser>(); });

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

void InterfaceClient::exit() {
    // Use exit_eventfd_ to trigger epoll to exit interface client
    uint64_t u = 1;
    write(exit_eventfd_, &u, sizeof(uint64_t));
}

int InterfaceClient::sendExitMsg() {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace([this] {
        // Stringify a ExitMsg into string
        std::unique_ptr<ExitMsg> exit_msg_ptr = std::make_unique<ExitMsg>();
        auto parser = this->parser_factory_.createParser("ExitMsg");
        std::string str = parser->stringify(std::move(exit_msg_ptr));

        std::printf("[Client %d] Send ExitMsg to all %lu clients.\n", this->client_id_, this->ip_port_pairs_.size() - 1);
        for (int i = 1; i < this->connect_sockfds_.size(); i++) {
            this->sendMsg(i, str);
        }
    });
    lock.unlock();
    cond_var_.notify_one();
    return 0;
}

int InterfaceClient::sendTransferTransReq(int client_id, int sender_id, int receiver_id, int amount) {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace([this, client_id, sender_id, receiver_id, amount] {
        // Stringify a TransferTransReq into string
        std::printf("[Client %d] Send TransferTransReq to client %d: %d pays %d $%d.\n", this->client_id_, client_id, sender_id, receiver_id, amount);
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
        std::printf("[Client %d] Send BalanceTransReq to client %d.\n", this->client_id_, client_id);
        std::unique_ptr<BalanceTransReq> balance_trans_req_ptr = std::make_unique<BalanceTransReq>(0);
        auto parser = this->parser_factory_.createParser("BalanceTransReq");
        std::string str = parser->stringify(std::move(balance_trans_req_ptr));

        this->sendMsg(client_id, str);
    });
    lock.unlock();
    cond_var_.notify_one();

    return 0;
}