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
    std::printf("\033[32m[Help] For 1 + 3 clients model:\033[0m\n");
    std::printf("\033[32m[Help] ----id 0, interface client;\033[0m\n");
    std::printf("\033[32m[Help] ----id 1, lamport client;\033[0m\n");
    std::printf("\033[32m[Help] ----id 2, lamport client;\033[0m\n");
    std::printf("\033[32m[Help] ----id 3, lamport client;\033[0m\n");
    char command[20];
    while (true) {
        std::printf("\033[32m[Input] Enter command (transfer <client_id> <sender> <receiver> <amount> or balance <client_id>):\033[0m\n");
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
                std::printf("\033[31m[Error] Invalid transfer command format.\033[0m\n");
                // Clear input buffer
                while (std::getchar() != '\n');
            }
        } else if (std::strcmp(command, "balance") == 0) {
            int client_id;
            if (std::scanf("%d", &client_id) == 1) {
                interface_client_ptr->sendBalanceTransReq(client_id);
            } else {
                std::printf("\033[31m[Error] Invalid balance command format.\033[0m\n");
                // Clear input buffer
                while (std::getchar() != '\n');
            }
        } else if (std::strcmp(command, "exit") == 0) {
            interface_client_ptr->sendExitMsg();
            interface_client_ptr->exit();
            break;
        } else {
            std::printf("\033[31m[Error] Unknown command.\033[0m\n");
            // Clear input buffer
            while (std::getchar() != '\n');
        }
        // Clear the newline character after successful command
        while (std::getchar() != '\n');
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        _base_port = std::stoi(argv[1]);
        // Update port numbers in _ip_port_pairs
        for (size_t i = 0; i < _ip_port_pairs.size(); i++) {
            _ip_port_pairs[i].second = _base_port + i;
        }
    }

    // Create lamport clients with ids 1, 2, 3 in child processes
    std::printf("[Init] Creating lamport clients with ids 1, 2, 3 in child processes.\n");
    std::vector<pid_t> lamport_pids;
    for (int i = 1; i < _ip_port_pairs.size(); i++) {
        pid_t pid = fork();
        if (pid == 0) {
            auto lamport_client_ptr = LamportClient::getInstance();
            lamport_client_ptr->init(i, _balance_tb, _ip_port_pairs);
            return 0;
        }
        lamport_pids.push_back(pid);
    }

    // Create interface client with id 0 in main process
    std::printf("[Init] Creating interface client with id 0 in main process.\n");
    auto interface_client_ptr = InterfaceClient::getInstance();
    interface_client_ptr->init(0, _ip_port_pairs);

    // Input for interface client
    cmd(interface_client_ptr);

    // Wait for all child processes to finish
    for (pid_t pid : lamport_pids) {
        waitpid(pid, nullptr, 0);
    }
    exit(0);
}