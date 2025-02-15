#include "processors/requestMsgProcessor.hpp"
#include "clients/lamportServer.hpp"

int RequestMsgProcessor::process(std::unique_ptr<Msg>&& msg){
    auto lamport_client_ptr = LamportServer::getInstance();
    int local_lamport_clock = -1;
    
    RequestMsg* msg_raw = dynamic_cast<RequestMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][RequestMsgProcessor::process][Server %d] message does not fit in RequestMsg.\033[0m\n", lamport_client_ptr->client_id_);
    }
    std::unique_ptr<RequestMsg> msg_ptr(static_cast<RequestMsg*>(msg.release())); 

    // Update lamport clock by merging remote one
    local_lamport_clock = lamport_client_ptr->updateLamportClock(msg_ptr->lamport_clock);
    std::printf("[Server %d][lamport_clock %d] Receive RequestMsg from server %d: %d pays %d $%d.\n", lamport_client_ptr->client_id_, local_lamport_clock, msg_ptr->client_id, msg_ptr->sender_id, msg_ptr->receiver_id, msg_ptr->amount);

    // Lock transfer priority queue mutex
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->transfer_pq_mutex_);
    Transfer transfer{msg_ptr->sender_id, msg_ptr->receiver_id, msg_ptr->amount, msg_ptr->lamport_clock, msg_ptr->client_id};
    lamport_client_ptr->transfer_pq_.push(transfer);
    transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex

    // Get lamport clock
    local_lamport_clock = lamport_client_ptr->getLamportClock();

    // Generate ReplyMsg
    std::unique_ptr<ReplyMsg> reply_msg_ptr = std::make_unique<ReplyMsg>(local_lamport_clock, lamport_client_ptr->client_id_);

    // Stringify ReplyMsg
    std::printf("[Server %d][lamport_clock %d] Send ReplyMsg to server %d.\n", lamport_client_ptr->client_id_, reply_msg_ptr->lamport_clock, msg_ptr->client_id);
    auto parser = lamport_client_ptr->parser_factory_.createParser("ReplyMsg");
    std::string str = parser->stringify(std::move(reply_msg_ptr));

    // Send message back to server that send request
    lamport_client_ptr->sendMsg(msg_ptr->client_id, str);
    return 0;
}