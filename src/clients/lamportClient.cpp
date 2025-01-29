#include "clients/lamportClient.hpp"
#include "parsers/msgParser.hpp"
#include <csignal>

void sigintLamportHandler(int signum) {
    // Get lamport client instance
    auto lamport_client_ptr = LamportClient::getInstance();
    
    // Stop lamport client
    if (lamport_client_ptr) {
        lamport_client_ptr->terminate();
    }
    
    // Exit the program
    exit(signum);
}

void LamportClient::init(int client_id, const std::vector<int> balance_tb, const std::vector<std::pair<std::string, int>>& ip_port_pairs) {
    initSockConfigs(client_id, ip_port_pairs);

    blockchain_ = Blockchain();
    balance_tb_ = balance_tb;
    transfer_pq_ = {};
    replys_ = std::vector<bool>(balance_tb.size(), false);
    finishes_ = std::vector<bool>(balance_tb.size(), false);
    transfer_result_ = "INVALID";
    lamport_clock_ = 0;

    // Register parser for request/response of transfer/balance transaction, request/reply/release/finish message
    std::printf("[Client %d] Register parser: TransferTransReqParser, BalanceTransReqParser, TransferTransRspParser, BalanceTransRspParser, RequestMsgParser, ReplyMsgParser, ReleaseMsgParser, FinishMsgParser.\n", client_id);
    parser_factory_.registerParser("TransferTransReq", []() { return std::make_unique<TransferTransReqParser>(); });
    parser_factory_.registerParser("BalanceTransReq", []() { return std::make_unique<BalanceTransReqParser>(); });
    parser_factory_.registerParser("TransferTransRsp", []() { return std::make_unique<TransferTransRspParser>(); });
    parser_factory_.registerParser("BalanceTransRsp", []() { return std::make_unique<BalanceTransRspParser>(); });
    parser_factory_.registerParser("RequestMsg", []() { return std::make_unique<RequestMsgParser>(); });
    parser_factory_.registerParser("ReplyMsg", []() { return std::make_unique<ReplyMsgParser>(); });
    parser_factory_.registerParser("ReleaseMsg", []() { return std::make_unique<ReleaseMsgParser>(); });
    parser_factory_.registerParser("FinishMsg", []() { return std::make_unique<FinishMsgParser>(); });

    // Register processor for request of transfer/balance transanction, request/reply/release/finish message
    std::printf("[Client %d] Register processor: TransferTransReqProcessor, BalanceTransReqProcessor, RequestMsgProcessor, ReplyMsgProcessor, ReleaseMsgProcessor, FinishMsgProcessor.\n", client_id);
    processor_factory_.registerProcessor("TransferTransReq", []() { return std::make_unique<TransferTransReqProcessor>(); });
    processor_factory_.registerProcessor("BalanceTransReq", []() { return std::make_unique<BalanceTransReqProcessor>(); });
    processor_factory_.registerProcessor("RequestMsg", []() { return std::make_unique<RequestMsgProcessor>(); });
    processor_factory_.registerProcessor("ReplyMsg", []() { return std::make_unique<ReplyMsgProcessor>(); });
    processor_factory_.registerProcessor("ReleaseMsg", []() { return std::make_unique<ReleaseMsgProcessor>(); });
    processor_factory_.registerProcessor("FinishMsg", []() { return std::make_unique<FinishMsgProcessor>(); });

    start();

    // Register SIGINT handler
    std::printf("[Client %d] Register SIGINT handler: sigintLamportHandler().\n", client_id);
    signal(SIGINT, sigintLamportHandler);

    pause(); // Wait indefinitely until a signal is received
}

std::shared_ptr<LamportClient> LamportClient::getInstance() {
    static std::shared_ptr<LamportClient> client_ptr; // static ensures uniqueness
    if (!client_ptr) {
        client_ptr = std::make_shared<LamportClient>();
    }
    return client_ptr;
}

void LamportClient::terminate() {
    // Stop master and worker threads
    stop();
}

void LamportClient::updateLamportClock(int remote_lamport_clock) {
    // Lock lamport clock mutex of receiving
    std::unique_lock<std::mutex> lamport_clock_lock_receive(lamport_clock_mutex_);
    lamport_clock_ = std::max(lamport_clock_, remote_lamport_clock) + 1; // Lamport clock = max + 1
    lamport_clock_mutex_.unlock(); // Unlock lamport clock mutex of receiving
}

int LamportClient::getLamportClock() {
    // Lock lamport clock mutex of sending
    std::unique_lock<std::mutex> lamport_clock_lock_send(lamport_clock_mutex_);
    int local_lamport_clock = lamport_clock_;
    lamport_clock_++; // Lamport clock++
    lamport_clock_mutex_.unlock(); // Unlock lamport clock mutex of sending
    return local_lamport_clock;
}

void LamportClient::transfer(bool update_transfer_result) {
    Transfer transfer = transfer_pq_.top(); // Transfer priority queue is ensured not empty
    transfer_pq_.pop();
    if (balance_tb_[transfer.sender_id - 1] >= transfer.amount) { // Success, could transfer
        std::printf("[Client %d] client %d pays client %d $%d: SUCCESS\n", client_id_, transfer.sender_id, transfer.receiver_id, transfer.amount);
        blockchain_.addBlock(transfer.sender_id, transfer.receiver_id, transfer.amount);
        balance_tb_[transfer.sender_id - 1] -= transfer.amount;
        balance_tb_[transfer.receiver_id - 1] += transfer.amount;
        if (update_transfer_result) {
            transfer_result_ = "SUCCESS";
        }
    } else { // Fail, balance not enough
        std::printf("[Client %d] client %d pays client %d $%d: FAIL\n", client_id_, transfer.sender_id, transfer.receiver_id, transfer.amount);
        if (update_transfer_result) {
            transfer_result_ = "FAIL";
        }
    }
}