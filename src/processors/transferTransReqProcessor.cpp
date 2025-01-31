#include "processors/transferTransReqProcessor.hpp"
#include "clients/lamportClient.hpp"

int TransferTransReqProcessor::process(std::unique_ptr<Msg>&& msg){
    auto lamport_client_ptr = LamportClient::getInstance();
    int local_lamport_clock = -1;

    // Lock balance transaction mutex
    lamport_client_ptr->balance_mutex_.lock();

    TransferTransReq* msg_raw = dynamic_cast<TransferTransReq*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][TransferTransReqProcessor::process][Client %d] message does not fit in TransferTransReq.\033[0m\n", lamport_client_ptr->client_id_);
        throw std::bad_cast();
    }
    std::unique_ptr<TransferTransReq> msg_ptr(static_cast<TransferTransReq*>(msg.release()));
    std::printf("[Client %d] Receive TransferTransReq from client 0: %d pays %d $%d.\n", lamport_client_ptr->client_id_, msg_ptr->sender_id, msg_ptr->receiver_id, msg_ptr->amount);

    // Get local lamport clock
    local_lamport_clock = lamport_client_ptr->getLamportClock();

    // Lock transfer priority queue mutex
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->transfer_pq_mutex_);
    Transfer transfer{msg_ptr->sender_id, msg_ptr->receiver_id, msg_ptr->amount, local_lamport_clock, lamport_client_ptr->client_id_};
    lamport_client_ptr->transfer_pq_.push(transfer);
    transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex

    // Lock replys vector mutex
    std::unique_lock<std::mutex> replys_lock(lamport_client_ptr->replys_mutex_);
    lamport_client_ptr->replys_ = std::vector<bool>(lamport_client_ptr->balance_tb_.size(), false);
    lamport_client_ptr->replys_[lamport_client_ptr->client_id_ - 1] = true; // Current client is true
    replys_lock.unlock(); // Unlock replys vector mutex

    // Lock finishes vector mutex
    std::unique_lock<std::mutex> finishes_lock(lamport_client_ptr->finishes_mutex_);
    lamport_client_ptr->finishes_ = std::vector<bool>(lamport_client_ptr->balance_tb_.size(), false);
    lamport_client_ptr->finishes_[lamport_client_ptr->client_id_ - 1] = true; // Current client is true
    finishes_lock.unlock(); // Unlock finishes vector mutex

    for (int i = 1; i < lamport_client_ptr->connect_sockfds_.size(); i++) {
        if (i == lamport_client_ptr->client_id_) {
            continue;
        }

        // Generate RequestMsg
        std::unique_ptr<RequestMsg> request_msg_ptr = std::make_unique<RequestMsg>(transfer.sender_id, transfer.receiver_id, transfer.amount, transfer.lamport_clock, transfer.client_id);

        // Stringify RequestMsg
        std::printf("[Client %d][lamport_clock %d] Send RequestMsg to client %d: %d pays %d $%d.\n", lamport_client_ptr->client_id_, request_msg_ptr->lamport_clock, i, request_msg_ptr->sender_id, request_msg_ptr->receiver_id, request_msg_ptr->amount); 
        auto parser = lamport_client_ptr->parser_factory_.createParser("RequestMsg");
        std::string str = parser->stringify(std::move(request_msg_ptr));

        // Send message to client i
        lamport_client_ptr->sendMsg(i, str);
    }
    return 0;
}