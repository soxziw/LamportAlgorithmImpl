#include "clients/interface.hpp"


void Interface::init(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs) {
    initSockConfigs(client_id, ip_port_pairs);

    // Register parser for request/response of transfer/balance transaction
    std::printf("[Server %d] Register parser: TransferTransReqParser, BalanceTransReqParser, TransferTransRspParser, BalanceTransRspParser, ExitMsgParser.\n", client_id);
    parser_factory_.registerParser("TransferTransReq", []() { return std::make_unique<TransferTransReqParser>(); });
    parser_factory_.registerParser("BalanceTransReq", []() { return std::make_unique<BalanceTransReqParser>(); });
    parser_factory_.registerParser("TransferTransRsp", []() { return std::make_unique<TransferTransRspParser>(); });
    parser_factory_.registerParser("BalanceTransRsp", []() { return std::make_unique<BalanceTransRspParser>(); });
    parser_factory_.registerParser("ExitMsg", []() { return std::make_unique<ExitMsgParser>(); });

    // Register processor for response of transfer/balance transanction
    std::printf("[Server %d] Register processor: TransferTransRspProcessor, BalanceTransRspProcessor.\n", client_id);
    processor_factory_.registerProcessor("TransferTransRsp", []() { return std::make_unique<TransferTransRspProcessor>(); });
    processor_factory_.registerProcessor("BalanceTransRsp", []() { return std::make_unique<BalanceTransRspProcessor>(); });
    
    // Start master and worker threads
    start();
}

std::shared_ptr<Interface> Interface::getInstance() {
    static std::shared_ptr<Interface> client_ptr; // static ensures uniqueness
    if (!client_ptr) {
        client_ptr = std::make_shared<Interface>();
    }
    return client_ptr;
}

void Interface::exit() {
    // Use exit_eventfd_ to trigger epoll to exit interface
    uint64_t u = 1;
    write(exit_eventfd_, &u, sizeof(uint64_t));
}

int Interface::sendExitMsg() {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace([this] {
        // Stringify a ExitMsg into string
        std::unique_ptr<ExitMsg> exit_msg_ptr = std::make_unique<ExitMsg>();
        auto parser = this->parser_factory_.createParser("ExitMsg");
        std::string str = parser->stringify(std::move(exit_msg_ptr));

        std::printf("[Server %d] Send ExitMsg to all %lu clients.\n", this->client_id_, this->ip_port_pairs_.size() - 1);
        for (int i = 1; i < this->connect_sockfds_.size(); i++) {
            this->sendMsg(i, str);
        }
    });
    lock.unlock();
    cond_var_.notify_one();
    return 0;
}

int Interface::sendTransferTransReq(int client_id, int sender_id, int receiver_id, int amount) {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace([this, client_id, sender_id, receiver_id, amount] {
        // Stringify a TransferTransReq into string
        std::printf("[Server %d] Send TransferTransReq to server %d: %d pays %d $%d.\n", this->client_id_, client_id, sender_id, receiver_id, amount);
        std::unique_ptr<TransferTransReq> transfer_trans_req_ptr = std::make_unique<TransferTransReq>(sender_id, receiver_id, amount, 0);
        auto parser = this->parser_factory_.createParser("TransferTransReq");
        std::string str = parser->stringify(std::move(transfer_trans_req_ptr));

        this->sendMsg(client_id, str);
    });
    lock.unlock();
    cond_var_.notify_one();
    return 0;
}

int Interface::sendBalanceTransReq(int client_id) {
    // Add task to the queue for worker threads
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.emplace([this, client_id] {
        // Stringify a BalanceTransReq into string
        std::printf("[Server %d] Send BalanceTransReq to server %d.\n", this->client_id_, client_id);
        std::unique_ptr<BalanceTransReq> balance_trans_req_ptr = std::make_unique<BalanceTransReq>(0);
        auto parser = this->parser_factory_.createParser("BalanceTransReq");
        std::string str = parser->stringify(std::move(balance_trans_req_ptr));

        this->sendMsg(client_id, str);
    });
    lock.unlock();
    cond_var_.notify_one();

    return 0;
}