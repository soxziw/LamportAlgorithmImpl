#pragma once
#include "clients/lamportClient.hpp"
#include <memory>

/**
 * updateLamportClock() - merge local lamport clock with remote lamport clock on receiving message.
 *
 * @param lamport_client_ptr
 * @param remote_lamport_clock
 */
void updateLamportClock(std::shared_ptr<LamportClient>& lamport_client_ptr, int remote_lamport_clock);

/**
 * getLamportClock() - get a new local lamport clock on sending message.
 *
 * @param lamport_client_ptr
 */
int getLamportClock(std::shared_ptr<LamportClient>& lamport_client_ptr);

/**
 * transfer() - implement top transfer.
 *
 * @param lamport_client_ptr
 * @param update_transfer_result: update transfer result of current client or not. not update on releasing.
 */
void transfer(std::shared_ptr<LamportClient>& lamport_client_ptr,  bool update_transfer_result);

/**
 * sendMsg() - sending message to target client.
 *
 * @param client_ptr
 * @param target_client_id
 * @param str
 */
void sendMsg(std::shared_ptr<Client> client_ptr, int target_client_id, std::string& str);