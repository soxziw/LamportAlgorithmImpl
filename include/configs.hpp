#pragma once

#include <string>
#include <vector>
#include <map>

int _base_port = 8000;
/**
 * _ip_port_pairs - ip-port pairs for clients to build up socket connections.
 *
 * The client 0 is the client working as the interface, rest of clients are the clients using lamport algorithm.
 */
std::vector<std::pair<std::string, int>> _ip_port_pairs = {
    {"127.0.0.1", _base_port}, // interface client
    {"127.0.0.1", _base_port + 1}, // lamport clients
    {"127.0.0.1", _base_port + 2}, 
    {"127.0.0.1", _base_port + 3}
};

/**
 * _balance_tb - balance table initializaiton.
 *
 * The starting point of each lamport clients.
 */
std::vector<int> _balance_tb = {
    10, 
    10, 
    10
};