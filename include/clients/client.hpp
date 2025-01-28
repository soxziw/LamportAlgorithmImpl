#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/epoll.h>
#include <functional>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory>
#include "msg.hpp"
#include "parsers/parserFactory.hpp"
#include "processors/processorFactory.hpp"

/**
 * Client - base class of clients.
 */
class Client : public std::enable_shared_from_this<Client> {
public:
    /**
     * getClientId() - get client_id_.
     */
    int& getClientId() {
        return client_id_;
    }

    /**
     * getKeepRunning() - get keep_running_.
     */
    bool& getKeepRunning() {
        return keep_running_;
    }

    /**
     * getParserFactory() - get parser_factory_.
     */ 
    ParserFactory& getParserFactory() {
        return parser_factory_;
    }

    /**
     * getProcessorFactory() - get processor_factory_.
     */
    ProcessorFactory& getProcessorFactory() {
        return processor_factory_;
    }

    /**
     * getConnectSockfds() - get connect_sockfds_.
     */
    std::vector<int>& getConnectSockfds() {
        return connect_sockfds_;
    }

    /**
     * getMutex() - get mutex_.
     */
    std::mutex& getMutex() {
        return mutex_;
    }

    /**
     * getCondVar() - get cond_var_.
     */
    std::condition_variable& getCondVar() {
        return cond_var_;
    }

    /**
     * getTaskQueue() - get task_queue_.
     */
    std::queue<std::function<void()>>& getTaskQueue() {
        return task_queue_;
    }

protected:
    /**
     * initSockConfigs() - init socket configurations.
     *
     * @param client_id: id of current client.
     * @param ip_port_pairs: ip-port pairs for clients to build up socket connections.
     */
    void initSockConfigs(int client_id, const std::vector<std::pair<std::string, int>>& ip_port_pairs);
    
    /**
     * start() - start processing.
     *
     * Create master thread to build up socket with other clients;
     * create worker thread to process message received from other clients.
     */
    void start();
    
    /**
     * stop() - stop processing.
     *
     * Signal workers to stop and join threads.
     */
    void stop();

private:
    /**
     * process() - processing.
     *
     * @param str: message received from socket.
     * Signal workers to stop and join threads.
     */
    int process(const std::string& str);

    /**
     * masterThreadFunc() - function that run on master thread.
     */
    void masterThreadFunc();
    
    /**
     * handleClient() - handle message from other client.
     *
     * @param client_sock
     */
    void handleClient(int client_sock);

    /**
     * workerThreadFunc() - function that run on worker thread.
     */
    void workerThreadFunc();

    ParserFactory parser_factory_; /* factory of parser */
    ProcessorFactory processor_factory_; /* factory of processor */

    int client_id_;
    bool keep_running_; /* to terminate master and worker threads */
    std::vector<std::pair<std::string, int>> ip_port_pairs_;
    std::vector<int> listen_sockfds_; /* to build up socket connections */
    std::vector<int> connect_sockfds_; /* to send messages */
    std::vector<int> accepted_sockfds_; /* to receive messages */

    int epoll_fd_;
    int num_workers_ = 5; /* default number of workers */

    /**
     * Producer-consumer model for master and worker threads.
     */
    std::thread master_thread_; /* producer */
    std::vector<std::thread> worker_threads_; /* consumer */
    std::queue<std::function<void()>> task_queue_;

    std::mutex mutex_;
    std::condition_variable cond_var_;
};
