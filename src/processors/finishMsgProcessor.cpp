#include "processors/finishMsgProcessor.hpp"
#include "clients/lamportServer.hpp"

int FinishMsgProcessor::process(std::unique_ptr<Msg>&& msg){
    auto lamport_client_ptr = LamportServer::getInstance();
    int local_lamport_clock = -1;

    FinishMsg* msg_raw = dynamic_cast<FinishMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][FinishMsgProcessor::process][Server %d] message does not fit in FinishMsg.\033[0m\n", lamport_client_ptr->client_id_);
    }
    std::unique_ptr<FinishMsg> msg_ptr(static_cast<FinishMsg*>(msg.release()));

    // Update lamport clock by merging remote one
    local_lamport_clock = lamport_client_ptr->updateLamportClock(msg_ptr->lamport_clock);
    std::printf("[Server %d][lamport_clock %d] Receive FinishMsg from server %d.\n", lamport_client_ptr->client_id_, local_lamport_clock, msg_ptr->client_id);

    // Lock finishes vector mutex
    std::unique_lock<std::mutex> finishes_lock(lamport_client_ptr->finishes_mutex_);
    lamport_client_ptr->finishes_[msg_ptr->client_id - 1] = true; // Finish server is true
    for (int i = 0; i < lamport_client_ptr->finishes_.size(); i++) {
        if (!lamport_client_ptr->finishes_[i]) { // Not all finish
            finishes_lock.unlock(); // Unlock finishes vector mutex
            return 0;
        }
    }
    finishes_lock.unlock(); // Unlock finishes vector mutex

    // Generate TransferTransRsp
    std::unique_ptr<TransferTransRsp> transfer_trans_rsp_ptr = std::make_unique<TransferTransRsp>(lamport_client_ptr->transfer_result_, lamport_client_ptr->client_id_);

    // Stringify TransferTransRsp
    std::printf("[Server %d] Send TransferTransRsp to server 0.\n", lamport_client_ptr->client_id_); 
    auto parser = lamport_client_ptr->parser_factory_.createParser("TransferTransRsp");
    std::string str = parser->stringify(std::move(transfer_trans_rsp_ptr));

    // Send message to server 0
    lamport_client_ptr->sendMsg(0, str);
    lamport_client_ptr->balance_mutex_.unlock();
    return 0;
}