#pragma once

#include <string>
#include <vector>

/**
 * Operation - transfer operation.
 *
 * - sender_id: sender of the transfer.
 * - receiver_id: receiver of the transfer.
 * - amount: amount of the transfer.
 * - opr: "<sender_id> pays <receiver_id> $<amount>"
 */
struct Operation {
    int sender_id;
    int receiver_id;
    int amount;
    std::string opr;
};

/**
 * Block - block on chain.
 *
 * - operation: transfer operation.
 * - prev_hash: hash of previous block.
 * - hash: hash of "<prev_hash><operation>".
 */
struct Block {
    Operation operation;
    std::string prev_hash;
    std::string hash;
};

/**
 * Blockchain - blockchain of the balance table.
 */
class Blockchain {
public:
    /**
    * addBlock() - add new block onto the blockchain.
    * 
    * @param sender_id
    * @param receiver_id
    * @param amount
    * Given transfer transaction, add a new block to the end of blockchain.
    * Calculate the hash of new block based on hash of previous block and current operation.
    */
    void addBlock(int sender_id, int receiver_id, int amount);

    /**
    * printBlockchain() - print the whole blockchain from begin to end.
    * 
    * Print operation, hash of previous block, hash of current block of each block.
    */
    void printBlockchain();

private:
    /**
    * calculateHash() - calculate SHA256 hash of "<prev_hash><operation>".
    */
    std::string calculateHash(const std::string& prev_hash, const std::string& opr);

    std::vector<Block> blockchain_; /* vector of block, as the chain of block. */
};