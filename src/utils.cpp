#include "utils.hpp"

void updateLamportClock(std::shared_ptr<LamportClient>& lamport_client_ptr, int remote_lamport_clock) {
    // Lock lamport clock mutex of receiving
    std::unique_lock<std::mutex> lamport_clock_lock_receive(lamport_client_ptr->getLamportClockMutex());
    lamport_client_ptr->getLamportClock() = std::max(lamport_client_ptr->getLamportClock(), remote_lamport_clock) + 1; // Lamport clock = max + 1
    lamport_clock_lock_receive.unlock(); // Unlock lamport clock mutex of receiving
}

int getLamportClock(std::shared_ptr<LamportClient>& lamport_client_ptr) {
    // Lock lamport clock mutex of sending
    std::unique_lock<std::mutex> lamport_clock_lock_send(lamport_client_ptr->getLamportClockMutex());
    int local_lamport_clock = lamport_client_ptr->getLamportClock();
    lamport_client_ptr->getLamportClock()++; // Lamport clock++
    lamport_clock_lock_send.unlock(); // Unlock lamport clock mutex of sending
    return local_lamport_clock;
}

void transfer(std::shared_ptr<LamportClient>& lamport_client_ptr, bool update_transfer_result) {
    Transfer transfer = lamport_client_ptr->getTransferPq().top(); // Transfer priority queue is ensured not empty
    lamport_client_ptr->getTransferPq().pop();
    if (lamport_client_ptr->getBalanceTb()[transfer.sender_id - 1] >= transfer.amount) { // Success, could transfer
        std::printf("[Client %d] client %d pays client %d $%d: SUCCESS\n", lamport_client_ptr->getClientId(), transfer.sender_id, transfer.receiver_id, transfer.amount);
        lamport_client_ptr->getBlockchain().addBlock(transfer.sender_id, transfer.receiver_id, transfer.amount);
        lamport_client_ptr->getBalanceTb()[transfer.sender_id - 1] -= transfer.amount;
        lamport_client_ptr->getBalanceTb()[transfer.receiver_id - 1] += transfer.amount;
        if (update_transfer_result) {
            lamport_client_ptr->getTransferResult() = "SUCCESS";
        }
    } else { // Fail, balance not enough
        std::printf("[Client %d] client %d pays client %d $%d: FAIL\n", lamport_client_ptr->getClientId(), transfer.sender_id, transfer.receiver_id, transfer.amount);
        if (update_transfer_result) {
            lamport_client_ptr->getTransferResult() = "FAIL";
        }
    }
}

void sendMsg(std::shared_ptr<Client> client_ptr, int target_client_id, std::string& str) {
    // Send message
    if (client_ptr->getConnectSockfds()[target_client_id] == -1) {
        // ERROR not connected
        std::printf("[ERROR][Client %d] Socket connection to client %d not set up.\n", client_ptr->getClientId(), target_client_id);
    }
    if (send(client_ptr->getConnectSockfds()[target_client_id], str.c_str(), str.size(), 0) < 0) {
        // ERROR
        std::printf("[ERROR][Client %d] Send RequestMsg to client %d fail, data: %s.\n", client_ptr->getClientId(), target_client_id, str.c_str());
    }
}