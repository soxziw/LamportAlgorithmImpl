#include "blockchain.hpp"
#include "clients/lamportClient.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

void Blockchain::addBlock(int sender_id, int receiver_id, int amount) {
    std::string prev_hash = blockchain_.empty() ? "0" : blockchain_.back().hash;
    std::string opr = std::to_string(sender_id) + " pays " + std::to_string(receiver_id) + " $" + std::to_string(amount);
    std::string hash = calculateHash(prev_hash, opr);
    blockchain_.push_back(Block{Operation{sender_id, receiver_id, amount, opr}, prev_hash, hash});
}

void Blockchain::printBlockchain() {
    for (int i = 0; i < blockchain_.size(); i++) {
        std::printf("[Client %d] Block %d. Operation: %s; Previous Hash: %s; Hash: %s\n", LamportClient::getInstance()->client_id_, i, blockchain_[i].operation.opr.c_str(), blockchain_[i].prev_hash.c_str(), blockchain_[i].hash.c_str());
    }
}

std::string Blockchain::calculateHash(const std::string& prev_hash, const std::string& opr) {
    std::string input = prev_hash + opr;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input.c_str(), input.size(), hash);

    char result[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(result + i * 2, "%02x", hash[i]);
    }
    return result;
}