#include "processors/finishMsgProcessor.hpp"
#include "clients/lamportClient.hpp"
#include "utils.hpp"

int FinishMsgProcessor::process(std::unique_ptr<Msg>&& msg){
    auto lamport_client_ptr = LamportClient::getInstance();

    std::printf("[Client %d] Process FinishMsg.\n", lamport_client_ptr->getClientId());

    FinishMsg* msg_raw = dynamic_cast<FinishMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][FinishMsgProcessor::process][Client %d] message does not fit in FinishMsg.\n", lamport_client_ptr->getClientId());
        throw std::bad_cast();
    }
    std::unique_ptr<FinishMsg> msg_ptr(static_cast<FinishMsg*>(msg.release()));

    // Update lamport clock by merging remote one
    updateLamportClock(lamport_client_ptr, msg_ptr->lamport_clock);

    // Lock finishes vector mutex
    std::unique_lock<std::mutex> finishes_lock(lamport_client_ptr->getFinishsMutex());
    lamport_client_ptr->getFinishs()[msg_ptr->client_id - 1] = true; // Finish client is true
    for (int i = 0; i < lamport_client_ptr->getFinishs().size(); i++) {
        if (!lamport_client_ptr->getFinishs()[i]) { // Not all finish
            finishes_lock.unlock(); // Unlock finishes vector mutex
            return 0;
        }
    }
    finishes_lock.unlock(); // Unlock finishes vector mutex

    // Generate TransferTransRsp
    std::unique_ptr<TransferTransRsp> transfer_trans_rsp_ptr = std::make_unique<TransferTransRsp>(lamport_client_ptr->getTransferResult(), lamport_client_ptr->getClientId());

    // Stringify TransferTransRsp
    std::printf("[Client %d] Send TransferTransRsp to client 0.\n", lamport_client_ptr->getClientId()); 
    auto parser = lamport_client_ptr->getParserFactory().createParser("TransferTransRsp");
    std::string str = parser->stringify(std::move(transfer_trans_rsp_ptr));

    // Send message to client 0
    sendMsg(lamport_client_ptr, 0, str);
    lamport_client_ptr->getBalanceMutex().unlock();
    return 0;
}