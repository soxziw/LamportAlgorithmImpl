#include "processors/releaseMsgProcessor.hpp"
#include "clients/lamportClient.hpp"
#include "utils.hpp"

int ReleaseMsgProcessor::process(std::unique_ptr<Msg>&& msg){
    auto lamport_client_ptr = LamportClient::getInstance();

    std::printf("[Client %d] Process ReleaseMsg.\n", lamport_client_ptr->getClientId());

    ReleaseMsg* msg_raw = dynamic_cast<ReleaseMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][ReleaseMsgProcessor::process][Client %d] message does not fit in ReleaseMsg.\n", lamport_client_ptr->getClientId());
        throw std::bad_cast();
    }
    std::unique_ptr<ReleaseMsg> msg_ptr(static_cast<ReleaseMsg*>(msg.release()));
    
    // Update lamport clock by merging remote one
    updateLamportClock(lamport_client_ptr, msg_ptr->lamport_clock);

    // Lock transfer priority queue mutex of releasing 
    std::unique_lock<std::mutex> transfer_pq_lock_release(lamport_client_ptr->getTransferPqMutex());
    if ((!lamport_client_ptr->getTransferPq().empty()) && (lamport_client_ptr->getTransferPq().top().client_id == msg_ptr->client_id)) { // Double check release transfer is in highest priority
        // Implement transfer but not update result
        transfer(lamport_client_ptr, false);
        transfer_pq_lock_release.unlock(); // Unlock transfer priority queue mutex of aligning

        // Get lamport clock
        int local_lamport_clock = getLamportClock(lamport_client_ptr);

        // Generate FinishMsg
        std::unique_ptr<FinishMsg> finish_msg_ptr = std::make_unique<FinishMsg>(local_lamport_clock, lamport_client_ptr->getClientId());

        // Stringify RequestMsg
        std::printf("[Client %d][lamport_clock %d] Send FinishMsg to client %d.\n", lamport_client_ptr->getClientId(), finish_msg_ptr->lamport_clock, msg_ptr->client_id); 
        auto parser = lamport_client_ptr->getParserFactory().createParser("FinishMsg");
        std::string str = parser->stringify(std::move(finish_msg_ptr));

        // Send message back to client that send release
        sendMsg(lamport_client_ptr, msg_ptr->client_id, str);
    } else {
        // ERROR
        std::printf("[ERROR][ReleaseMsgProcessor::process][Client %d] Release transfer(client %d) is not on the top(client %d).\n", lamport_client_ptr->getClientId(), msg_ptr->client_id, lamport_client_ptr->getTransferPq().top().client_id);
        transfer_pq_lock_release.unlock(); // Unlock transfer priority queue mutex of aligning
    }

    // Lock replys vector mutex
    std::unique_lock<std::mutex> replys_lock(lamport_client_ptr->getReplysMutex());
    for (int i = 0; i < lamport_client_ptr->getReplys().size(); i++) {
        if (!lamport_client_ptr->getReplys()[i]) { // Not all reply
            replys_lock.unlock(); // Unlock replys vector mutex
            return 0;
        }
    }
    replys_lock.unlock();

    // Lock transfer priority queue mutex for new transfer
    std::unique_lock<std::mutex> transfer_pq_lock(lamport_client_ptr->getTransferPqMutex());
    if ((!lamport_client_ptr->getTransferPq().empty()) && (lamport_client_ptr->getTransferPq().top().client_id == lamport_client_ptr->getClientId())) { // Transfer of current client is in highest priority 
        // Implement tranfer and update result
        transfer(lamport_client_ptr, true);
        transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex for trigger new transfer

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
        transfer_pq_lock.unlock(); // Unlock transfer priority queue mutex for trigger new transfer
    }
    return 0;
}