#include "processors/releaseMsgProcessor.hpp"
#include "clients/lamportServer.hpp"

int ReleaseMsgProcessor::process(std::unique_ptr<Msg>&& msg){
    auto lamport_client_ptr = LamportServer::getInstance();
    int local_lamport_clock = -1;

    ReleaseMsg* msg_raw = dynamic_cast<ReleaseMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][ReleaseMsgProcessor::process][Server %d] message does not fit in ReleaseMsg.\033[0m\n", lamport_client_ptr->client_id_);
    }
    std::unique_ptr<ReleaseMsg> msg_ptr(static_cast<ReleaseMsg*>(msg.release()));
    
    // Update lamport clock by merging remote one
    local_lamport_clock = lamport_client_ptr->updateLamportClock(msg_ptr->lamport_clock);
    std::printf("[Server %d][lamport_clock %d] Receive ReleaseMsg from server %d.\n", lamport_client_ptr->client_id_, local_lamport_clock, msg_ptr->client_id);

    // Lock transfer priority queue mutex of releasing 
    std::unique_lock<std::mutex> transfer_pq_lock_release(lamport_client_ptr->transfer_pq_mutex_);
    if ((!lamport_client_ptr->transfer_pq_.empty()) && (lamport_client_ptr->transfer_pq_.top().client_id == msg_ptr->client_id)) { // Double check release transfer is in highest priority
        // Implement transfer but not update result
        lamport_client_ptr->transfer(false);
        transfer_pq_lock_release.unlock(); // Unlock transfer priority queue mutex of aligning

        // Get lamport clock
        local_lamport_clock = lamport_client_ptr->getLamportClock();

        // Generate FinishMsg
        std::unique_ptr<FinishMsg> finish_msg_ptr = std::make_unique<FinishMsg>(local_lamport_clock, lamport_client_ptr->client_id_);

        // Stringify RequestMsg
        std::printf("[Server %d][lamport_clock %d] Send FinishMsg to server %d.\n", lamport_client_ptr->client_id_, finish_msg_ptr->lamport_clock, msg_ptr->client_id); 
        auto parser = lamport_client_ptr->parser_factory_.createParser("FinishMsg");
        std::string str = parser->stringify(std::move(finish_msg_ptr));

        // Send message back to server that send release
        lamport_client_ptr->sendMsg(msg_ptr->client_id, str);
    } else {
        // ERROR
        std::printf("\033[31m[Error][ReleaseMsgProcessor::process][Server %d] Release transfer(server %d) is not on the top(server %d).\033[0m\n", lamport_client_ptr->client_id_, msg_ptr->client_id, lamport_client_ptr->transfer_pq_.top().client_id);
        transfer_pq_lock_release.unlock(); // Unlock transfer priority queue mutex of aligning
    }

    // Lock replys vector mutex
    std::unique_lock<std::mutex> replys_lock(lamport_client_ptr->replys_mutex_);
    for (int i = 0; i < lamport_client_ptr->replys_.size(); i++) {
        if (!lamport_client_ptr->replys_[i]) { // Not all reply
            replys_lock.unlock(); // Unlock replys vector mutex
            return 0;
        }
    }
    replys_lock.unlock();

    // Lock transfer priority queue mutex for new transfer
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->transfer_pq_mutex_);
    if ((!lamport_client_ptr->transfer_pq_.empty()) && (lamport_client_ptr->transfer_pq_.top().client_id == lamport_client_ptr->client_id_)) { // Transfer of current server is in highest priority 
        // Implement tranfer and update result
        lamport_client_ptr->transfer(true);
        transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex for trigger new transfer

        // Get lamport clock
        local_lamport_clock = lamport_client_ptr->getLamportClock();

        for (int i = 1; i < lamport_client_ptr->connect_sockfds_.size(); i++) {
            if (i == lamport_client_ptr->client_id_) {
                continue;
            }

            // Generate ReleaseMsg
            std::unique_ptr<ReleaseMsg> release_msg_ptr = std::make_unique<ReleaseMsg>(local_lamport_clock, lamport_client_ptr->client_id_);

            // Stringify RequestMsg
            std::printf("[Server %d][lamport_clock %d] Send ReleaseMsg to server %d.\n", lamport_client_ptr->client_id_, release_msg_ptr->lamport_clock, i); 
            auto parser = lamport_client_ptr->parser_factory_.createParser("ReleaseMsg");
            std::string str = parser->stringify(std::move(release_msg_ptr));

            // Send message to server i
            lamport_client_ptr->sendMsg(i, str);
        }
    } else {
        transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex for trigger new transfer
    }
    return 0;
}