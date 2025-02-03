#include "processors/balanceTransRspProcessor.hpp"
#include "clients/interface.hpp"

int BalanceTransRspProcessor::process(std::unique_ptr<Msg>&& msg) {
    auto interface_client_ptr = Interface::getInstance();

    BalanceTransRsp* msg_raw = dynamic_cast<BalanceTransRsp*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][BalanceTransRspProcessor::process][Server %d] message does not fit in BalanceTransRsp.\033[0m\n", interface_client_ptr->client_id_);
    }
    std::unique_ptr<BalanceTransRsp> msg_ptr(static_cast<BalanceTransRsp*>(msg.release()));
    std::printf("[Server %d] Receive BalanceTransRsp from server %d.\n", interface_client_ptr->client_id_, msg_ptr->client_id);

    std::printf("\033[32m[BalanceTransRsp] From server %d:\033[0m", msg_ptr->client_id);
    for (const std::pair<int, int>& pair : msg_ptr->client_balance_pairs) {
        std::printf("\033[32m client_id: %d, balance: %d;\033[0m", pair.first, pair.second);
    }
    std::printf("\n");
    return 0;
}