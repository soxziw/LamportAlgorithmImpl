#include "processors/replyMsgProcessor.hpp"
#include "clients/lamportClient.hpp"

int ReplyMsgProcessor::process(std::unique_ptr<Msg>&& msg) {
    auto lamport_client_ptr = LamportClient::getInstance();
    int local_lamport_clock = -1;

    ReplyMsg* msg_raw = dynamic_cast<ReplyMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][ReplyMsgProcessor::process][Client %d] message does not fit in ReplyMsg.\033[0m\n", lamport_client_ptr->client_id_);
        throw std::bad_cast();
    }
    std::unique_ptr<ReplyMsg> msg_ptr(static_cast<ReplyMsg*>(msg.release()));
    
    // Update lamport clock by merging remote one
    local_lamport_clock = lamport_client_ptr->updateLamportClock(msg_ptr->lamport_clock);
    std::printf("[Client %d][lamport_clock %d] Receive ReplyMsg from client %d.\n", lamport_client_ptr->client_id_, local_lamport_clock, msg_ptr->client_id);

    // Lock replys vector mutex
    std::unique_lock<std::mutex> replys_lock(lamport_client_ptr->replys_mutex_);
    lamport_client_ptr->replys_[msg_ptr->client_id - 1] = true; // Reply client is true
    for (int i = 0; i < lamport_client_ptr->replys_.size(); i++) {
        if (!lamport_client_ptr->replys_[i]) { // Not all reply
            replys_lock.unlock(); // Unlock replys vector mutex
            return 0;
        }
    }
    replys_lock.unlock(); // Unlock replys vector mutex

    // Lock transfer priority queue mutex
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->transfer_pq_mutex_);
    if ((!lamport_client_ptr->transfer_pq_.empty()) && (lamport_client_ptr->transfer_pq_.top().client_id == lamport_client_ptr->client_id_)) { // Transfer of current client is in highest priority
        // Implement transfer and update result
        lamport_client_ptr->transfer(true);
        transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex

        // Get lamport clock
        local_lamport_clock = lamport_client_ptr->getLamportClock();

        for (int i = 1; i < lamport_client_ptr->connect_sockfds_.size(); i++) {
            if (i == lamport_client_ptr->client_id_) {
                continue;
            }

            // Generate ReleaseMsg
            std::unique_ptr<ReleaseMsg> release_msg_ptr = std::make_unique<ReleaseMsg>(local_lamport_clock, lamport_client_ptr->client_id_);

            // Stringify RequestMsg
            std::printf("[Client %d][lamport_clock %d] Send ReleaseMsg to client %d.\n", lamport_client_ptr->client_id_, release_msg_ptr->lamport_clock, i); 
            auto parser = lamport_client_ptr->parser_factory_.createParser("ReleaseMsg");
            std::string str = parser->stringify(std::move(release_msg_ptr));

            // Send message to client i
            lamport_client_ptr->sendMsg(i, str);
        }
    } else {
        transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex
    }
    return 0;
}