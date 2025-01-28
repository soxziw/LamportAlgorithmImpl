#include "processors/replyMsgProcessor.hpp"
#include "clients/lamportClient.hpp"
#include "utils.hpp"

int ReplyMsgProcessor::process(std::unique_ptr<Msg>&& msg) {
    auto lamport_client_ptr = LamportClient::getInstance();

    std::printf("[Client %d] Process ReplyMsg.\n", lamport_client_ptr->getClientId());

    ReplyMsg* msg_raw = dynamic_cast<ReplyMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][ReplyMsgProcessor::process][Client %d] message does not fit in ReplyMsg.\n", lamport_client_ptr->getClientId());
        throw std::bad_cast();
    }
    std::unique_ptr<ReplyMsg> msg_ptr(static_cast<ReplyMsg*>(msg.release()));
    
    // Update lamport clock by merging remote one
    updateLamportClock(lamport_client_ptr, msg_ptr->lamport_clock);

    // Lock replys vector mutex
    std::unique_lock<std::mutex> replys_lock(lamport_client_ptr->getReplysMutex());
    lamport_client_ptr->getReplys()[msg_ptr->client_id - 1] = true; // Reply client is true
    for (int i = 0; i < lamport_client_ptr->getReplys().size(); i++) {
        if (!lamport_client_ptr->getReplys()[i]) { // Not all reply
            replys_lock.unlock(); // Unlock replys vector mutex
            return 0;
        }
    }
    replys_lock.unlock(); // Unlock replys vector mutex

    // Lock transfer priority queue mutex
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->getTransferPqMutex());
    if (lamport_client_ptr->getTransferPq().top().client_id == lamport_client_ptr->getClientId()) { // Transfer of current client is in highest priority
        // Implement transfer and update result
        transfer(lamport_client_ptr, true);
        transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex

        // Get lamport clock
        int local_lamport_clock = getLamportClock(lamport_client_ptr);

        for (int i = 1; i < lamport_client_ptr->getConnectSockfds().size(); i++) {
            if (i == lamport_client_ptr->getClientId()) {
                continue;
            }

            // Generate ReleaseMsg
            std::unique_ptr<ReleaseMsg> release_msg_ptr = std::make_unique<ReleaseMsg>(local_lamport_clock, lamport_client_ptr->getClientId());

            // Stringify RequestMsg
            std::printf("[Client %d][lamport_clock %d] Send ReleaseMsg to client %d.\n", lamport_client_ptr->getClientId(), release_msg_ptr->lamport_clock, i); 
            auto parser = lamport_client_ptr->getParserFactory().createParser("ReleaseMsg");
            std::string str = parser->stringify(std::move(release_msg_ptr));

            // Send message to client i
            sendMsg(lamport_client_ptr, i, str);
        }
    } else {
        transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex
    }
    return 0;
}