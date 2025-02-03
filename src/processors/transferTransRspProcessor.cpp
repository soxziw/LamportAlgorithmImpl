#include "processors/transferTransRspProcessor.hpp"
#include "clients/interface.hpp"

int TransferTransRspProcessor::process(std::unique_ptr<Msg>&& msg){
    auto interface_client_ptr = Interface::getInstance();

    TransferTransRsp* msg_raw = dynamic_cast<TransferTransRsp*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][TransferTransRspProcessor::process][Server %d] message does not fit in TransferTransRsp.\033[0m\n", interface_client_ptr->client_id_);
    }
    std::unique_ptr<TransferTransRsp> msg_ptr(static_cast<TransferTransRsp*>(msg.release()));
    std::printf("[Server %d] Receive TransferTransRsp from server %d.\n", interface_client_ptr->client_id_, msg_ptr->client_id);
    
    if (msg_ptr->transfer_result == "SUCCESS") {
        std::printf("\033[32m[TransferTransRsp] From server %d: SUCCESS.\033[0m\n", msg_ptr->client_id);
    } else {
        std::printf("\033[31m[TransferTransRsp] From server %d: FAIL.\033[0m\n", msg_ptr->client_id);
    }
    return 0;
}