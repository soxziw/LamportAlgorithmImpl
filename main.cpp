#include "configs.hpp"
#include "clients/lamportClient.hpp"
#include "clients/interfaceClient.hpp"
#include <sys/wait.h>
#include <csignal>

/**
 * cmd - a while loop listening for command from terminal.
 *
 * @param interface_client_ptr
 * - Transfer transaction: transfer <client_id> <sender> <receiver> <amount>;
 * - Balance transation: balance <client_id>;
 * - Exit: exit;
 */
void cmd(std::shared_ptr<InterfaceClient> interface_client_ptr) {
    std::printf("[Main] For 1 + 3 clients model:\n");
    std::printf("[Main] ----id 0, interface client;\n");
    std::printf("[Main] ----id 1, lamport client;\n");
    std::printf("[Main] ----id 2, lamport client;\n");
    std::printf("[Main] ----id 3, lamport client;\n");
    char command[20];
    while (true) {
        std::printf("[Main] Enter command (transfer <client_id> <sender> <receiver> <amount> or balance <client_id>): \n");
        int result = std::scanf("%19s", command);
        
        if (result != 1) {
            // Clear input buffer if invalid input
            while (std::getchar() != '\n');
            continue;
        }
        
        if (std::strcmp(command, "transfer") == 0) {
            int client_id, sender_id, receiver_id, amount;
            if (std::scanf("%d %d %d %d", &client_id, &sender_id, &receiver_id, &amount) == 4) {
                interface_client_ptr->sendTransferTransReq(client_id, sender_id, receiver_id, amount);
            } else {
                std::printf("[Main] Invalid transfer command format.\n");
                // Clear input buffer
                while (std::getchar() != '\n');
            }
        } else if (std::strcmp(command, "balance") == 0) {
            int client_id;
            if (std::scanf("%d", &client_id) == 1) {
                interface_client_ptr->sendBalanceTransReq(client_id);
            } else {
                std::printf("[Main] Invalid balance command format.\n");
                // Clear input buffer
                while (std::getchar() != '\n');
            }
        } else if (std::strcmp(command, "exit") == 0) {
            break;
        } else {
            std::printf("[Main] Unknown command.\n");
            // Clear input buffer
            while (std::getchar() != '\n');
        }
        // Clear the newline character after successful command
        while (std::getchar() != '\n');
    }
}

int main() {
    // Create lamport clients with ids 1, 2, 3 in child processes
    std::printf("[Main] Creating lamport clients with ids 1, 2, 3 in child processes.\n");
    std::vector<pid_t> lamport_pids;
    for (int i = 1; i <= 3; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            auto lamport_client_ptr = LamportClient::getInstance();
            lamport_client_ptr->init(i, _balance_tb, _ip_port_pairs);
        }
        lamport_pids.push_back(pid);
    }

    // Create interface client with id 0 in main process
    std::printf("[Main] Creating interface client with id 0 in main process.\n");
    auto interface_client_ptr = InterfaceClient::getInstance();
    interface_client_ptr->init(0, _ip_port_pairs);

    // Input for interface client
    cmd(interface_client_ptr);

    // Stop all clients
    interface_client_ptr->terminate();
    for (pid_t pid : lamport_pids) {
        kill(pid, SIGINT);
    }

    // Wait for all child processes to finish
    for (pid_t pid : lamport_pids) {
        waitpid(pid, nullptr, 0);
    }
    return 0;
}