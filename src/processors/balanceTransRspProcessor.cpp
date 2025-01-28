#include "processors/balanceTransRspProcessor.hpp"
#include "clients/interfaceClient.hpp"

int BalanceTransRspProcessor::process(std::unique_ptr<Msg>&& msg) {
    auto interface_client_ptr = InterfaceClient::getInstance();

    std::printf("[Client %d] Process BalanceTransRsp.\n", interface_client_ptr->getClientId());

    BalanceTransRsp* msg_raw = dynamic_cast<BalanceTransRsp*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][BalanceTransRspProcessor::process][Client %d] message does not fit in BalanceTransRsp.\n", interface_client_ptr->getClientId());
        throw std::bad_cast();
    }
    std::unique_ptr<BalanceTransRsp> msg_ptr(static_cast<BalanceTransRsp*>(msg.release()));

    std::printf("[BalanceTransRsp] From client %d:", msg_ptr->client_id);
    for (const std::pair<int, int>& pair : msg_ptr->client_balance_pairs) {
        std::printf(" client_id: %d, balance: %d;", pair.first, pair.second);
    }
    std::printf("\n");
    return 0;
}