#pragma once

#include <string>
#include <vector>

/**
 * Msg - base class of messages.
 */
struct Msg {
    virtual ~Msg() = default;
};

/**
 * ExitMsg - exit and stop client.
 */
struct ExitMsg : public Msg {
    ExitMsg()
    : Msg() {}
};

/**
 * TransferTransReq - request of transfer transaction.
 *
 * - sender_id: sender's id.
 * - receiver_id: receivers'id.
 * - amount: amount of money to transfer.
 * - client_id: client that process this transaciton, receiver of this request.
 */
struct TransferTransReq : public Msg {
    int sender_id = -1;
    int receiver_id = -1;
    int amount = 0;
    int client_id = -1;

    TransferTransReq(int sender_id, int receiver_id, int amount, int client_id)
    : Msg(), sender_id(sender_id), receiver_id(receiver_id), amount(amount), client_id(client_id) {}
};

/**
 * TransferTransRsp - response of transfer transaction.
 *
 * - transfer_result: result of the transaction.
 * - client_id: client that process this transaciton, sender of this response.
 */
struct TransferTransRsp : public Msg {
    std::string transfer_result = "FAIL";
    int client_id = -1;

    TransferTransRsp(std::string transfer_result, int client_id)
    : Msg(), transfer_result(transfer_result), client_id(client_id) {}
};

/**
 * BalanceTransReq - request of balance transaction.
 *
 * - client_id: client that process this transaciton, receiver of this request.
 */
struct BalanceTransReq : public Msg {
    int client_id = -1;

    BalanceTransReq(int client_id) : Msg(), client_id(client_id) {}
};

/**
 * BalanceTransRsp - response of balance transaction.
 *
 * - client_balance_pairs: pairs of client id and balance.
 * - client_id: client that process this transaciton, sender of this response.
 */
struct BalanceTransRsp : public Msg {
    std::vector<std::pair<int, int>> client_balance_pairs = {};
    int client_id = -1;
    
    BalanceTransRsp(std::vector<std::pair<int, int>>& client_balance_pairs, int client_id)
    : Msg(), client_balance_pairs(client_balance_pairs), client_id(client_id) {}
};

/**
 * RequestMsg - request message.
 *
 * - sender_id: sender's id.
 * - receiver_id: receivers'id.
 * - amount: amount of money to transfer.
 * - lamport_clock: local lamport clock of the sender client.
 * - client_id: client that process the transaciton, sender of this request.
 */
struct RequestMsg : public Msg {
    int sender_id = -1;
    int receiver_id = -1;
    int amount = 0;
    int lamport_clock = -1;
    int client_id = -1;

    RequestMsg(int sender_id, int receiver_id, int amount, int lamport_clock, int client_id)
    : Msg(), sender_id(sender_id), receiver_id(receiver_id), amount(amount), lamport_clock(lamport_clock), client_id(client_id) {}
};

/**
 * ReplyMsg - reply message.
 *
 * - lamport_clock: local lamport clock of the sender client.
 * - client_id: sender of this reply.
 */
struct ReplyMsg : public Msg {
    int lamport_clock = -1;
    int client_id = -1;

    ReplyMsg(int lamport_clock, int client_id)
    : Msg(), lamport_clock(lamport_clock), client_id(client_id) {}
};

/**
 * ReleaseMsg - release message.
 *
 * - lamport_clock: local lamport clock of the sender client.
 * - client_id: client that process the transaciton, sender of this release.
 */
struct ReleaseMsg : public Msg {
    int lamport_clock = -1;
    int client_id = -1;

    ReleaseMsg(int lamport_clock, int client_id)
    : Msg(), lamport_clock(lamport_clock), client_id(client_id) {}
};

/**
 * FinishMsg - finish message.
 *
 * - lamport_clock: local lamport clock of the sender client.
 * - client_id: sender of this finish.
 */
struct FinishMsg : public Msg {
    int lamport_clock = -1;
    int client_id = -1;

    FinishMsg(int lamport_clock, int client_id)
    : Msg(), lamport_clock(lamport_clock), client_id(client_id) {}
};