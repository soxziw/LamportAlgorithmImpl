#include "processors/requestMsgProcessor.hpp"
#include "clients/lamportClient.hpp"
#include "utils.hpp"

int RequestMsgProcessor::process(std::unique_ptr<Msg>&& msg){
    auto lamport_client_ptr = LamportClient::getInstance();

    std::printf("[Client %d] Process RequestMsg.\n", lamport_client_ptr->getClientId());
    
    RequestMsg* msg_raw = dynamic_cast<RequestMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][RequestMsgProcessor::process][Client %d] message does not fit in RequestMsg.\n", lamport_client_ptr->getClientId());
        throw std::bad_cast();
    }
    std::unique_ptr<RequestMsg> msg_ptr(static_cast<RequestMsg*>(msg.release())); 

    // Update lamport clock by merging remote one
    updateLamportClock(lamport_client_ptr, msg_ptr->lamport_clock);

    // Lock transfer priority queue mutex
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->getTransferPqMutex());
    Transfer transfer{msg_ptr->sender_id, msg_ptr->receiver_id, msg_ptr->amount, msg_ptr->lamport_clock, msg_ptr->client_id};
    lamport_client_ptr->getTransferPq().push(transfer);
    transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex

    // Get lamport clock
    int local_lamport_clock = getLamportClock(lamport_client_ptr);

    // Generate ReplyMsg
    std::unique_ptr<ReplyMsg> reply_msg_ptr = std::make_unique<ReplyMsg>(local_lamport_clock, lamport_client_ptr->getClientId());

    // Stringify ReplyMsg
    std::printf("[Client %d][lamport_clock %d] Send ReplyMsg to client %d.\n", lamport_client_ptr->getClientId(), reply_msg_ptr->lamport_clock, msg_ptr->client_id);
    auto parser = lamport_client_ptr->getParserFactory().createParser("ReplyMsg");
    std::string str = parser->stringify(std::move(reply_msg_ptr));

    // Send message back to client that send request
    sendMsg(lamport_client_ptr, msg_ptr->client_id, str);
    return 0;
}