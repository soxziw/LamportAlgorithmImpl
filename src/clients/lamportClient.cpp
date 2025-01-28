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
    getParserFactory().registerParser("TransferTransReq", []() { return std::make_unique<TransferTransReqParser>(); });
    getParserFactory().registerParser("BalanceTransReq", []() { return std::make_unique<BalanceTransReqParser>(); });
    getParserFactory().registerParser("TransferTransRsp", []() { return std::make_unique<TransferTransRspParser>(); });
    getParserFactory().registerParser("BalanceTransRsp", []() { return std::make_unique<BalanceTransRspParser>(); });
    getParserFactory().registerParser("RequestMsg", []() { return std::make_unique<RequestMsgParser>(); });
    getParserFactory().registerParser("ReplyMsg", []() { return std::make_unique<ReplyMsgParser>(); });
    getParserFactory().registerParser("ReleaseMsg", []() { return std::make_unique<ReleaseMsgParser>(); });
    getParserFactory().registerParser("FinishMsg", []() { return std::make_unique<FinishMsgParser>(); });

    // Register processor for request of transfer/balance transanction, request/reply/release/finish message
    std::printf("[Client %d] Register processor: TransferTransReqProcessor, BalanceTransReqProcessor, RequestMsgProcessor, ReplyMsgProcessor, ReleaseMsgProcessor, FinishMsgProcessor.\n", client_id);
    getProcessorFactory().registerProcessor("TransferTransReq", []() { return std::make_unique<TransferTransReqProcessor>(); });
    getProcessorFactory().registerProcessor("BalanceTransReq", []() { return std::make_unique<BalanceTransReqProcessor>(); });
    getProcessorFactory().registerProcessor("RequestMsg", []() { return std::make_unique<RequestMsgProcessor>(); });
    getProcessorFactory().registerProcessor("ReplyMsg", []() { return std::make_unique<ReplyMsgProcessor>(); });
    getProcessorFactory().registerProcessor("ReleaseMsg", []() { return std::make_unique<ReleaseMsgProcessor>(); });
    getProcessorFactory().registerProcessor("FinishMsg", []() { return std::make_unique<FinishMsgProcessor>(); });

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