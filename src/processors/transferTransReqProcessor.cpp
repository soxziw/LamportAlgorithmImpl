#include "processors/transferTransReqProcessor.hpp"
#include "clients/lamportClient.hpp"
#include "utils.hpp"

int TransferTransReqProcessor::process(std::unique_ptr<Msg>&& msg){
    auto lamport_client_ptr = LamportClient::getInstance();

    // Lock balance transaction mutex
    std::printf("[Client %d] Process TransferTransReq.\n", lamport_client_ptr->getClientId());
    lamport_client_ptr->getBalanceMutex().lock();

    TransferTransReq* msg_raw = dynamic_cast<TransferTransReq*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][TransferTransReqProcessor::process][Client %d] message does not fit in TransferTransReq.\n", lamport_client_ptr->getClientId());
        throw std::bad_cast();
    }
    std::unique_ptr<TransferTransReq> msg_ptr(static_cast<TransferTransReq*>(msg.release()));

    // Get local lamport clock
    int local_lamport_clock = getLamportClock(lamport_client_ptr);

    // Lock transfer priority queue mutex
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->getTransferPqMutex());
    Transfer transfer{msg_ptr->sender_id, msg_ptr->receiver_id, msg_ptr->amount, local_lamport_clock, lamport_client_ptr->getClientId()};
    lamport_client_ptr->getTransferPq().push(transfer);
    transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex

    // Lock replys vector mutex
    std::unique_lock<std::mutex> replys_lock(lamport_client_ptr->getReplysMutex());
    lamport_client_ptr->getReplys() = std::vector<bool>(lamport_client_ptr->getBalanceTb().size(), true);
    lamport_client_ptr->getReplys()[lamport_client_ptr->getClientId() - 1] = true; // Current client is true
    replys_lock.unlock(); // Unlock replys vector mutex

    // Lock finishes vector mutex
    std::unique_lock<std::mutex> finishes_lock(lamport_client_ptr->getFinishesMutex());
    lamport_client_ptr->getFinishes() = std::vector<bool>(lamport_client_ptr->getBalanceTb().size(), true);
    lamport_client_ptr->getFinishes()[lamport_client_ptr->getClientId() - 1] = true; // Current client is true
    finishes_lock.unlock(); // Unlock finishes vector mutex

    for (int i = 1; i < lamport_client_ptr->getConnectSockfds().size(); i++) {
        if (i == lamport_client_ptr->getClientId()) {
            continue;
        }

        // Generate RequestMsg
        std::unique_ptr<RequestMsg> request_msg_ptr = std::make_unique<RequestMsg>(transfer.sender_id, transfer.receiver_id, transfer.amount, transfer.lamport_clock, transfer.client_id);

        // Stringify RequestMsg
        std::printf("[Client %d][lamport_clock %d] Send RequestMsg to client %d. sender_id: %d, receiver_id: %d, amount: %d.\n", lamport_client_ptr->getClientId(), request_msg_ptr->lamport_clock, i, request_msg_ptr->sender_id, request_msg_ptr->receiver_id, request_msg_ptr->amount); 
        auto parser = lamport_client_ptr->getParserFactory().createParser("RequestMsg");
        std::string str = parser->stringify(std::move(request_msg_ptr));

        // Send message to client i
        sendMsg(lamport_client_ptr, i, str);
    }
    return 0;
}