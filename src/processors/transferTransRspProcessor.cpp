#include "processors/transferTransRspProcessor.hpp"
#include "clients/interfaceClient.hpp"

int TransferTransRspProcessor::process(std::unique_ptr<Msg>&& msg){
    auto interface_client_ptr = InterfaceClient::getInstance();

    std::printf("[Client %d] Process TransferTransRsp.\n", interface_client_ptr->client_id_);

    TransferTransRsp* msg_raw = dynamic_cast<TransferTransRsp*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][TransferTransRspProcessor::process][Client %d] message does not fit in TransferTransRsp.\n", interface_client_ptr->client_id_);
        throw std::bad_cast();
    }
    std::unique_ptr<TransferTransRsp> msg_ptr(static_cast<TransferTransRsp*>(msg.release()));
    
    std::printf("[TransferTransRsp] From client %d: %s.\n", msg_ptr->client_id, msg_ptr->transfer_result.c_str());
    return 0;
}